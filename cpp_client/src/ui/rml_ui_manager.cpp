/**
 * @file rml_ui_manager.cpp
 * @brief RmlUi-based UI manager implementation for EVE OFFLINE.
 *
 * When built with -DUSE_RMLUI=ON, this uses RmlUi's official GLFW platform
 * and OpenGL 3 renderer backends for production-quality UI rendering.
 *
 * When USE_RMLUI is not defined, all methods compile as no-op stubs so the
 * rest of the client can link without the RmlUi dependency.
 */

#ifdef USE_RMLUI

#include "ui/rml_ui_manager.h"
#include "core/entity.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Platform_GLFW.h>
#include <RmlUi_Renderer_GL3.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace UI {

// UTF-8 encoding of the superscript 3 character (³) for m³ display
static constexpr const char* CUBIC_METER_SUFFIX = " m\xc2\xb3";

// ============================================================================
// RmlUiManager — uses official RmlUi GL3 + GLFW backends
// ============================================================================

RmlUiManager::RmlUiManager() = default;

RmlUiManager::~RmlUiManager() {
    Shutdown();
}

bool RmlUiManager::Initialize(GLFWwindow* window, const std::string& resourcePath) {
    if (initialized_) return true;

    if (!window) {
        std::cerr << "[RmlUiManager] Error: invalid GLFW window\n";
        return false;
    }

    window_ = window;
    resourcePath_ = resourcePath;

    // Initialize the GL3 function loader used by RmlUi's renderer
    Rml::String gl_message;
    if (!RmlGL3::Initialize(&gl_message)) {
        std::cerr << "[RmlUiManager] Error: RmlGL3::Initialize failed: " << gl_message << "\n";
        return false;
    }
    std::cout << "[RmlUiManager] " << gl_message << "\n";

    // Create the system (platform) and render interfaces from RmlUi's backends
    systemInterface_ = std::make_unique<SystemInterface_GLFW>();
    systemInterface_->SetWindow(window);
    renderInterface_ = std::make_unique<RenderInterface_GL3>();

    if (!*renderInterface_) {
        std::cerr << "[RmlUiManager] Error: RenderInterface_GL3 creation failed\n";
        RmlGL3::Shutdown();
        return false;
    }

    // Register interfaces with RmlUi core
    Rml::SetSystemInterface(systemInterface_.get());
    Rml::SetRenderInterface(renderInterface_.get());

    // Initialize RmlUi core
    if (!Rml::Initialise()) {
        std::cerr << "[RmlUiManager] Error: Rml::Initialise() failed\n";
        RmlGL3::Shutdown();
        return false;
    }

    // Get framebuffer dimensions
    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    renderInterface_->SetViewport(fbWidth, fbHeight);

    // Create main context
    context_ = Rml::CreateContext("main", Rml::Vector2i(fbWidth, fbHeight));
    if (!context_) {
        std::cerr << "[RmlUiManager] Error: failed to create RmlUi context\n";
        Rml::Shutdown();
        RmlGL3::Shutdown();
        return false;
    }

    // Initialize the visual debugger (toggled with F8)
    Rml::Debugger::Initialise(context_);

    // Load fonts for UI rendering.
    // RmlUi requires at least one font to render text. The first font loaded
    // as a fallback will be used when no other font matches.
    struct FontCandidate {
        const char* path;
        bool fallback;
    };

    // Try to load Lato (EVE Online's UI font), then common system fallbacks
    FontCandidate regularFonts[] = {
        {"/usr/share/fonts/truetype/lato/Lato-Regular.ttf", true},
        {"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", true},
        {"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", true},
        {"/usr/share/fonts/TTF/DejaVuSans.ttf", true},
        {"C:\\Windows\\Fonts\\segoeui.ttf", true},
        {"C:\\Windows\\Fonts\\arial.ttf", true},
    };

    bool fontLoaded = false;
    for (const auto& font : regularFonts) {
        if (Rml::LoadFontFace(font.path, font.fallback)) {
            std::cout << "[RmlUiManager] Loaded font: " << font.path << "\n";
            fontLoaded = true;
            break;
        }
    }

    // Try to load bold variant
    const char* boldFonts[] = {
        "/usr/share/fonts/truetype/lato/Lato-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "C:\\Windows\\Fonts\\segoeuib.ttf",
        "C:\\Windows\\Fonts\\arialbd.ttf",
    };
    for (const auto* boldPath : boldFonts) {
        if (Rml::LoadFontFace(boldPath, false)) {
            std::cout << "[RmlUiManager] Loaded bold font: " << boldPath << "\n";
            break;
        }
    }

    if (!fontLoaded) {
        std::cerr << "[RmlUiManager] Warning: no system fonts found, text may not render\n";
    }

    // Load core documents
    if (!LoadDocuments()) {
        std::cerr << "[RmlUiManager] Warning: some documents failed to load\n";
    }

    initialized_ = true;
    std::cout << "[RmlUiManager] Initialized (" << fbWidth << "x" << fbHeight << ")\n";
    return true;
}

void RmlUiManager::Shutdown() {
    if (!initialized_) return;

    documents_.clear();

    if (context_) {
        Rml::Debugger::Shutdown();
        Rml::RemoveContext("main");
        context_ = nullptr;
    }

    Rml::Shutdown();

    renderInterface_.reset();
    systemInterface_.reset();

    RmlGL3::Shutdown();

    initialized_ = false;
    std::cout << "[RmlUiManager] Shutdown complete\n";
}

// ---- Per-frame methods ----

void RmlUiManager::ProcessInput() {
    // Input is forwarded via Handle* callbacks wired by the application.
}

void RmlUiManager::Update() {
    if (!initialized_ || !context_) return;

    // Update HUD elements with current ship data
    UpdateHudElements();

    // Update target list display
    // (only rebuilds DOM when targets change via Set/Remove/ClearTargets)

    context_->Update();
}

void RmlUiManager::BeginFrame() {
    if (!initialized_ || !renderInterface_) return;
    renderInterface_->BeginFrame();
}

void RmlUiManager::Render() {
    if (!initialized_ || !context_) return;
    context_->Render();
}

void RmlUiManager::EndFrame() {
    if (!initialized_ || !renderInterface_) return;
    renderInterface_->EndFrame();
}

// ---- Input forwarding ----

void RmlUiManager::HandleKey(int key, int action, int mods) {
    if (!initialized_ || !context_) return;
    active_mods_ = mods;
    RmlGLFW::ProcessKeyCallback(context_, key, action, mods);
}

void RmlUiManager::HandleChar(unsigned int codepoint) {
    if (!initialized_ || !context_) return;
    RmlGLFW::ProcessCharCallback(context_, codepoint);
}

void RmlUiManager::HandleCursorPos(double xpos, double ypos) {
    if (!initialized_ || !context_ || !window_) return;
    RmlGLFW::ProcessCursorPosCallback(context_, window_, xpos, ypos, active_mods_);
}

void RmlUiManager::HandleMouseButton(int button, int action, int mods) {
    if (!initialized_ || !context_) return;
    active_mods_ = mods;
    RmlGLFW::ProcessMouseButtonCallback(context_, button, action, mods);
}

void RmlUiManager::HandleScroll(double yoffset, int mods) {
    if (!initialized_ || !context_) return;
    active_mods_ = mods;
    RmlGLFW::ProcessScrollCallback(context_, yoffset, active_mods_);
}

void RmlUiManager::HandleFramebufferSize(int width, int height) {
    if (!initialized_ || !context_ || !renderInterface_) return;
    renderInterface_->SetViewport(width, height);
    RmlGLFW::ProcessFramebufferSizeCallback(context_, width, height);
}

// ---- Ship Status ----

void RmlUiManager::SetShipStatus(const ShipStatusData& data) {
    shipData_ = data;
}

void RmlUiManager::SetShieldPercent(float pct) {
    shipData_.shield_pct = pct;
}

void RmlUiManager::SetArmorPercent(float pct) {
    shipData_.armor_pct = pct;
}

void RmlUiManager::SetHullPercent(float pct) {
    shipData_.hull_pct = pct;
}

void RmlUiManager::SetCapacitorPercent(float pct) {
    shipData_.capacitor_pct = pct;
}

void RmlUiManager::SetVelocity(float velocity) {
    shipData_.velocity = velocity;
}

void RmlUiManager::SetMaxVelocity(float maxVelocity) {
    shipData_.max_velocity = maxVelocity;
}

void RmlUiManager::SetShipStatus(float shieldPct, float armorPct, float hullPct,
                                  float capPct, float velocity, float maxVelocity) {
    shipData_.shield_pct = shieldPct;
    shipData_.armor_pct = armorPct;
    shipData_.hull_pct = hullPct;
    shipData_.capacitor_pct = capPct;
    shipData_.velocity = velocity;
    shipData_.max_velocity = maxVelocity;
}

// ---- Target List ----

void RmlUiManager::SetTarget(const std::string& id, const std::string& name,
                              float shieldPct, float armorPct, float hullPct,
                              float distance, bool isHostile, bool isActive) {
    // Update existing target or add new one
    for (auto& t : targets_) {
        if (t.id == id) {
            t.name = name;
            t.shieldPct = shieldPct;
            t.armorPct = armorPct;
            t.hullPct = hullPct;
            t.distance = distance;
            t.isHostile = isHostile;
            t.isActive = isActive;
            UpdateTargetListElements();
            return;
        }
    }

    TargetInfo info;
    info.id = id;
    info.name = name;
    info.shieldPct = shieldPct;
    info.armorPct = armorPct;
    info.hullPct = hullPct;
    info.distance = distance;
    info.isHostile = isHostile;
    info.isActive = isActive;
    targets_.push_back(info);
    UpdateTargetListElements();
}

void RmlUiManager::RemoveTarget(const std::string& id) {
    targets_.erase(
        std::remove_if(targets_.begin(), targets_.end(),
            [&id](const TargetInfo& t) { return t.id == id; }),
        targets_.end());
    UpdateTargetListElements();
}

void RmlUiManager::ClearTargets() {
    targets_.clear();
    UpdateTargetListElements();
}

// ---- Overview ----

void RmlUiManager::UpdateOverviewData(
    const std::unordered_map<std::string, std::shared_ptr<::eve::Entity>>& entities,
    const glm::vec3& playerPos)
{
    if (!initialized_ || !context_) return;

    auto it = documents_.find("overview");
    if (it == documents_.end() || !it->second) return;

    auto* body = it->second->GetElementById("overview-body");
    if (!body) return;

    // Clear existing rows
    while (body->HasChildNodes()) {
        body->RemoveChild(body->GetFirstChild());
    }

    // Build complete RML string first, then set it once to avoid O(n²) re-parsing
    std::string allRowsRml;
    for (const auto& [id, entity] : entities) {
        if (!entity) continue;

        // Get entity position and compute distance
        auto pos = entity->getPosition();
        float dx = pos.x - playerPos.x;
        float dy = pos.y - playerPos.y;
        float dz = pos.z - playerPos.z;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

        // Format distance
        char distStr[32];
        if (dist > 1000.0f) {
            std::snprintf(distStr, sizeof(distStr), "%.1f km", dist / 1000.0f);
        } else {
            std::snprintf(distStr, sizeof(distStr), "%.0f m", dist);
        }

        // Determine standing
        std::string standingClass = "neutral";
        std::string textClass = "text-neutral";
        std::string entityType = entity->getShipType();

        if (entityType == "npc" || entityType == "hostile") {
            standingClass = "hostile";
            textClass = "text-hostile";
        } else if (entityType == "friendly" || entityType == "fleet") {
            standingClass = "friendly";
            textClass = "text-friendly";
        }

        // Apply overview filter
        if (!overviewFilter_.empty() && overviewFilter_ != "all") {
            if (overviewFilter_ == "hostile" && standingClass != "hostile") continue;
            if (overviewFilter_ == "friendly" && standingClass != "friendly") continue;
            if (overviewFilter_ == "neutral" && standingClass != "neutral") continue;
        }

        allRowsRml +=
            "<tr>"
            "<td><span class=\"standing-dot " + standingClass + "\"></span></td>"
            "<td class=\"" + textClass + "\">" + entity->getShipName() + "</td>"
            "<td class=\"distance\">" + distStr + "</td>"
            "<td class=\"entity-type\">" + entityType + "</td>"
            "</tr>";
    }

    body->SetInnerRML(allRowsRml);
}

void RmlUiManager::SetOverviewFilter(const std::string& filter) {
    overviewFilter_ = filter;
    // Re-render the overview with the new filter applied.
    // The actual filtering is done in UpdateOverviewData() using overviewFilter_.
}

// ---- Panel Visibility ----

void RmlUiManager::SetDocumentVisible(const std::string& name, bool visible) {
    auto it = documents_.find(name);
    if (it != documents_.end() && it->second) {
        if (visible) {
            it->second->Show();
        } else {
            it->second->Hide();
        }
    }
}

bool RmlUiManager::IsDocumentVisible(const std::string& name) const {
    auto it = documents_.find(name);
    if (it != documents_.end() && it->second) {
        return it->second->IsVisible();
    }
    return false;
}

void RmlUiManager::ToggleDocument(const std::string& name) {
    SetDocumentVisible(name, !IsDocumentVisible(name));
}

// ---- Combat Log ----

void RmlUiManager::AddCombatLogMessage(const std::string& message) {
    auto it = documents_.find("ship_hud");
    if (it == documents_.end() || !it->second) return;

    auto* logBody = it->second->GetElementById("combat-log-body");
    if (!logBody) return;

    // Create a new message div
    std::string msgRml = "<div class=\"message\">" + message + "</div>";
    logBody->SetInnerRML(logBody->GetInnerRML() + msgRml);

    // Scroll to bottom
    logBody->SetScrollTop(logBody->GetScrollHeight());
}

// ---- Internal helpers ----

bool RmlUiManager::LoadDocuments() {
    if (!context_) return false;

    struct DocInfo {
        std::string name;
        std::string path;
        bool showByDefault;
    };

    std::vector<DocInfo> docs = {
        {"ship_hud",     resourcePath_ + "/rml/ship_hud.rml",     true},
        {"overview",     resourcePath_ + "/rml/overview.rml",      true},
        {"fitting",      resourcePath_ + "/rml/fitting.rml",       false},
        {"target_list",  resourcePath_ + "/rml/target_list.rml",   true},
        {"inventory",    resourcePath_ + "/rml/inventory.rml",     false},
        {"market",       resourcePath_ + "/rml/market.rml",        false},
        {"mission",      resourcePath_ + "/rml/mission.rml",       false},
        {"dscan",        resourcePath_ + "/rml/dscan.rml",         false},
        {"neocom",       resourcePath_ + "/rml/neocom.rml",        true},
        {"chat",         resourcePath_ + "/rml/chat.rml",          false},
        {"context_menu", resourcePath_ + "/rml/context_menu.rml",  false},
        {"drone_bay",    resourcePath_ + "/rml/drone_bay.rml",     false},
    };

    bool allOk = true;
    for (const auto& info : docs) {
        auto* doc = context_->LoadDocument(info.path);
        if (doc) {
            documents_[info.name] = doc;
            if (info.showByDefault) {
                doc->Show();
            }
            std::cout << "[RmlUiManager] Loaded document: " << info.name
                      << " (" << info.path << ")\n";
        } else {
            std::cerr << "[RmlUiManager] Failed to load: " << info.path << "\n";
            allOk = false;
        }
    }

    return allOk;
}

void RmlUiManager::UpdateHudElements() {
    auto it = documents_.find("ship_hud");
    if (it == documents_.end() || !it->second) return;

    auto* doc = it->second;

    // Update health bar widths via style property
    auto setBarWidth = [&](const char* id, float pct) {
        auto* el = doc->GetElementById(id);
        if (el) {
            char style[64];
            std::snprintf(style, sizeof(style), "width: %.1f%%", pct * 100.0f);
            el->SetAttribute("style", Rml::String(style));
        }
    };

    setBarWidth("shield-fill", shipData_.shield_pct);
    setBarWidth("armor-fill", shipData_.armor_pct);
    setBarWidth("hull-fill", shipData_.hull_pct);
    setBarWidth("cap-fill", shipData_.capacitor_pct);

    // Update speed text
    auto* speedVal = doc->GetElementById("speed-value");
    if (speedVal) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0f", shipData_.velocity);
        speedVal->SetInnerRML(buf);
    }

    auto* maxSpeedVal = doc->GetElementById("max-speed-value");
    if (maxSpeedVal) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0f", shipData_.max_velocity);
        maxSpeedVal->SetInnerRML(buf);
    }

    // Update HP summary text
    auto* shieldText = doc->GetElementById("shield-text");
    if (shieldText) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.0f", shipData_.shield_pct * 100.0f);
        shieldText->SetInnerRML(buf);
    }

    auto* armorText = doc->GetElementById("armor-text");
    if (armorText) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.0f", shipData_.armor_pct * 100.0f);
        armorText->SetInnerRML(buf);
    }

    auto* hullText = doc->GetElementById("hull-text");
    if (hullText) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.0f", shipData_.hull_pct * 100.0f);
        hullText->SetInnerRML(buf);
    }

    auto* capText = doc->GetElementById("cap-text");
    if (capText) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.0f", shipData_.capacitor_pct * 100.0f);
        capText->SetInnerRML(buf);
    }
}

void RmlUiManager::UpdateTargetListElements() {
    auto it = documents_.find("target_list");
    if (it == documents_.end() || !it->second) return;

    auto* container = it->second->GetElementById("target-cards");
    if (!container) return;

    // Build all target card RML
    std::string cardsRml;
    for (const auto& t : targets_) {
        std::string cardClass = "target-card";
        std::string nameClass = "target-name";
        if (t.isActive) cardClass += " active";
        if (t.isHostile) {
            cardClass += " hostile";
            nameClass += " hostile";
        } else {
            cardClass += " friendly";
            nameClass += " friendly";
        }

        // Format distance
        char distStr[32];
        if (t.distance > 1000.0f) {
            std::snprintf(distStr, sizeof(distStr), "%.1f km", t.distance / 1000.0f);
        } else {
            std::snprintf(distStr, sizeof(distStr), "%.0f m", t.distance);
        }

        char shieldW[16], armorW[16], hullW[16];
        std::snprintf(shieldW, sizeof(shieldW), "%.1f%%", t.shieldPct * 100.0f);
        std::snprintf(armorW, sizeof(armorW), "%.1f%%", t.armorPct * 100.0f);
        std::snprintf(hullW, sizeof(hullW), "%.1f%%", t.hullPct * 100.0f);

        cardsRml +=
            "<div class=\"" + cardClass + "\">"
            "<div class=\"" + nameClass + "\">" + t.name + "</div>"
            "<div class=\"target-health\">"
            "<div class=\"target-bar-track\"><div class=\"target-bar-fill target-shield-fill\" style=\"width: " + shieldW + ";\"></div></div>"
            "<div class=\"target-bar-track\"><div class=\"target-bar-fill target-armor-fill\" style=\"width: " + armorW + ";\"></div></div>"
            "<div class=\"target-bar-track\"><div class=\"target-bar-fill target-hull-fill\" style=\"width: " + hullW + ";\"></div></div>"
            "</div>"
            "<div class=\"target-distance\">" + distStr + "</div>"
            "</div>";
    }

    container->SetInnerRML(cardsRml);
}

void RmlUiManager::UpdateInventoryData(
    const std::vector<std::string>& names,
    const std::vector<std::string>& types,
    const std::vector<int>& quantities,
    const std::vector<float>& volumes,
    float capacityUsed, float capacityMax)
{
    if (!initialized_ || !context_) return;

    auto it = documents_.find("inventory");
    if (it == documents_.end() || !it->second) return;

    // Update capacity display
    auto* capText = it->second->GetElementById("capacity-text");
    if (capText) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.1f / %.1f%s", capacityUsed, capacityMax, CUBIC_METER_SUFFIX);
        capText->SetInnerRML(buf);
    }

    auto* capFill = it->second->GetElementById("capacity-fill");
    if (capFill && capacityMax > 0.0f) {
        float pct = (capacityUsed / capacityMax) * 100.0f;
        char style[64];
        std::snprintf(style, sizeof(style), "width: %.1f%%", pct);
        capFill->SetAttribute("style", Rml::String(style));
    }

    // Update item table
    auto* body = it->second->GetElementById("inventory-body");
    if (!body) return;

    std::string rowsRml;
    size_t count = std::min({names.size(), types.size(), quantities.size(), volumes.size()});
    for (size_t i = 0; i < count; ++i) {
        char volStr[32];
        std::snprintf(volStr, sizeof(volStr), "%.1f%s", volumes[i] * quantities[i], CUBIC_METER_SUFFIX);
        char qtyStr[16];
        std::snprintf(qtyStr, sizeof(qtyStr), "%d", quantities[i]);

        rowsRml +=
            "<tr>"
            "<td class=\"item-name\">" + names[i] + "</td>"
            "<td class=\"item-type\">" + types[i] + "</td>"
            "<td class=\"item-qty\">" + qtyStr + "</td>"
            "<td class=\"item-volume\">" + volStr + "</td>"
            "</tr>";
    }

    body->SetInnerRML(rowsRml);
}

void RmlUiManager::UpdateDScanResults(
    const std::vector<std::string>& names,
    const std::vector<std::string>& types,
    const std::vector<float>& distances)
{
    if (!initialized_ || !context_) return;

    auto it = documents_.find("dscan");
    if (it == documents_.end() || !it->second) return;

    // Update results count
    size_t count = std::min({names.size(), types.size(), distances.size()});
    auto* countEl = it->second->GetElementById("results-count");
    if (countEl) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%zu", count);
        countEl->SetInnerRML(buf);
    }

    // Update results table
    auto* body = it->second->GetElementById("dscan-body");
    if (!body) return;

    std::string rowsRml;
    for (size_t i = 0; i < count; ++i) {
        char distStr[32];
        if (distances[i] < 1.0f) {
            constexpr float KM_PER_AU = 149597.871f;
            std::snprintf(distStr, sizeof(distStr), "%.0f km", distances[i] * KM_PER_AU);
        } else {
            std::snprintf(distStr, sizeof(distStr), "%.2f AU", distances[i]);
        }

        rowsRml +=
            "<tr>"
            "<td class=\"result-type\">" + types[i] + "</td>"
            "<td class=\"result-name\">" + names[i] + "</td>"
            "<td class=\"result-distance\">" + distStr + "</td>"
            "</tr>";
    }

    body->SetInnerRML(rowsRml);
}

void RmlUiManager::UpdateDroneBayData(
    const std::vector<DroneRmlInfo>& spaceDrones,
    const std::vector<DroneRmlInfo>& bayDrones,
    int usedBandwidth, int maxBandwidth,
    float bayUsed, float bayCapacity)
{
    if (!initialized_ || !context_) return;

    auto it = documents_.find("drone_bay");
    if (it == documents_.end() || !it->second) return;

    auto* doc = it->second;

    // Update bandwidth bar
    auto* bwText = doc->GetElementById("bandwidth-text");
    if (bwText) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%d / %d Mbit/s", usedBandwidth, maxBandwidth);
        bwText->SetInnerRML(buf);
    }

    auto* bwFill = doc->GetElementById("bandwidth-fill");
    if (bwFill && maxBandwidth > 0) {
        float pct = (static_cast<float>(usedBandwidth) / maxBandwidth) * 100.0f;
        char style[64];
        std::snprintf(style, sizeof(style), "width: %.1f%%", pct);
        bwFill->SetAttribute("style", Rml::String(style));
    }

    // Update bay capacity bar
    auto* bayText = doc->GetElementById("bay-capacity-text");
    if (bayText) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.1f / %.1f%s", bayUsed, bayCapacity, CUBIC_METER_SUFFIX);
        bayText->SetInnerRML(buf);
    }

    auto* bayFill = doc->GetElementById("bay-capacity-fill");
    if (bayFill && bayCapacity > 0.0f) {
        float pct = (bayUsed / bayCapacity) * 100.0f;
        char style[64];
        std::snprintf(style, sizeof(style), "width: %.1f%%", pct);
        bayFill->SetAttribute("style", Rml::String(style));
    }

    // Update space drone count
    auto* spaceCount = doc->GetElementById("space-drone-count");
    if (spaceCount) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%zu", spaceDrones.size());
        spaceCount->SetInnerRML(buf);
    }

    // Update bay drone count
    auto* bayCount = doc->GetElementById("bay-drone-count");
    if (bayCount) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%zu", bayDrones.size());
        bayCount->SetInnerRML(buf);
    }

    // Build space drones list
    auto* spaceBody = doc->GetElementById("space-drones-body");
    if (spaceBody) {
        std::string rowsRml;
        for (const auto& d : spaceDrones) {
            std::string hpClass = "drone-hp-fill";
            if (d.healthPct < 0.25f) hpClass += " critical";
            else if (d.healthPct < 0.50f) hpClass += " damaged";

            std::string statusClass = d.engaging ? "drone-status engaging" : "drone-status idle";
            std::string statusText = d.engaging ? "Engaging" : "Idle";

            char hpW[16];
            std::snprintf(hpW, sizeof(hpW), "%.1f%%", d.healthPct * 100.0f);

            rowsRml +=
                "<div class=\"drone-row\">"
                "<span class=\"drone-name\">" + d.name + "</span>"
                "<span class=\"drone-type\">" + d.type + "</span>"
                "<div class=\"drone-hp-track\"><div class=\"" + hpClass + "\" style=\"width: " + hpW + ";\"></div></div>"
                "<span class=\"" + statusClass + "\">" + statusText + "</span>"
                "</div>";
        }
        spaceBody->SetInnerRML(rowsRml);
    }

    // Build bay drones list
    auto* bayBody = doc->GetElementById("bay-drones-body");
    if (bayBody) {
        std::string rowsRml;
        for (const auto& d : bayDrones) {
            std::string hpClass = "drone-hp-fill";
            if (d.healthPct < 0.25f) hpClass += " critical";
            else if (d.healthPct < 0.50f) hpClass += " damaged";

            char hpW[16];
            std::snprintf(hpW, sizeof(hpW), "%.1f%%", d.healthPct * 100.0f);

            rowsRml +=
                "<div class=\"drone-row\">"
                "<span class=\"drone-name\">" + d.name + "</span>"
                "<span class=\"drone-type\">" + d.type + "</span>"
                "<div class=\"drone-hp-track\"><div class=\"" + hpClass + "\" style=\"width: " + hpW + ";\"></div></div>"
                "<span class=\"drone-status idle\">Bay</span>"
                "</div>";
        }
        bayBody->SetInnerRML(rowsRml);
    }
}

bool RmlUiManager::WantsMouseInput() const {
    if (!initialized_ || !context_) return false;
    return context_->GetHoverElement() != nullptr;
}

bool RmlUiManager::WantsKeyboardInput() const {
    if (!initialized_ || !context_) return false;
    return context_->GetFocusElement() != nullptr;
}

} // namespace UI

#else // !USE_RMLUI

// ============================================================================
// Stub implementation when RmlUi is not available
// ============================================================================

#include "ui/rml_ui_manager.h"
#include <iostream>

namespace UI {

RmlUiManager::RmlUiManager() = default;
RmlUiManager::~RmlUiManager() = default;

bool RmlUiManager::Initialize(GLFWwindow*, const std::string&) {
    std::cout << "[RmlUiManager] RmlUi not enabled (build with -DUSE_RMLUI=ON)\n";
    return false;
}

void RmlUiManager::Shutdown() {}
void RmlUiManager::ProcessInput() {}
void RmlUiManager::Update() {}
void RmlUiManager::BeginFrame() {}
void RmlUiManager::Render() {}
void RmlUiManager::EndFrame() {}

void RmlUiManager::HandleKey(int, int, int) {}
void RmlUiManager::HandleChar(unsigned int) {}
void RmlUiManager::HandleCursorPos(double, double) {}
void RmlUiManager::HandleMouseButton(int, int, int) {}
void RmlUiManager::HandleScroll(double, int) {}
void RmlUiManager::HandleFramebufferSize(int, int) {}

void RmlUiManager::SetShipStatus(const ShipStatusData&) {}
void RmlUiManager::SetShieldPercent(float) {}
void RmlUiManager::SetArmorPercent(float) {}
void RmlUiManager::SetHullPercent(float) {}
void RmlUiManager::SetCapacitorPercent(float) {}
void RmlUiManager::SetVelocity(float) {}
void RmlUiManager::SetMaxVelocity(float) {}
void RmlUiManager::SetShipStatus(float, float, float, float, float, float) {}

void RmlUiManager::SetTarget(const std::string&, const std::string&,
                              float, float, float, float, bool, bool) {}
void RmlUiManager::RemoveTarget(const std::string&) {}
void RmlUiManager::ClearTargets() {}

void RmlUiManager::UpdateOverviewData(
    const std::unordered_map<std::string, std::shared_ptr<::eve::Entity>>&,
    const glm::vec3&) {}
void RmlUiManager::SetOverviewFilter(const std::string&) {}

void RmlUiManager::SetDocumentVisible(const std::string&, bool) {}
bool RmlUiManager::IsDocumentVisible(const std::string&) const { return false; }
void RmlUiManager::ToggleDocument(const std::string&) {}
void RmlUiManager::AddCombatLogMessage(const std::string&) {}
void RmlUiManager::UpdateInventoryData(
    const std::vector<std::string>&, const std::vector<std::string>&,
    const std::vector<int>&, const std::vector<float>&, float, float) {}
void RmlUiManager::UpdateDScanResults(
    const std::vector<std::string>&, const std::vector<std::string>&,
    const std::vector<float>&) {}
void RmlUiManager::UpdateDroneBayData(
    const std::vector<DroneRmlInfo>&, const std::vector<DroneRmlInfo>&,
    int, int, float, float) {}

bool RmlUiManager::WantsMouseInput() const { return false; }
bool RmlUiManager::WantsKeyboardInput() const { return false; }

} // namespace UI

#endif // USE_RMLUI
