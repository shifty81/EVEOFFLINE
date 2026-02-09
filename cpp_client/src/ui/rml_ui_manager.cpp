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

    // Setup GLFW input callbacks
    SetupInputCallbacks();

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
    // Input is handled via GLFW callbacks set up in SetupInputCallbacks()
}

void RmlUiManager::Update() {
    if (!initialized_ || !context_) return;

    // Update HUD elements with current ship data
    UpdateHudElements();

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

// ---- Ship Status ----

void RmlUiManager::SetShipStatus(const RmlShipData& data) {
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
    (void)id; (void)name;
    (void)shieldPct; (void)armorPct; (void)hullPct;
    (void)distance; (void)isHostile; (void)isActive;
    // TODO: Dynamically update target list DOM elements
}

void RmlUiManager::RemoveTarget(const std::string& id) {
    (void)id;
}

void RmlUiManager::ClearTargets() {
}

// ---- Overview ----

void RmlUiManager::UpdateOverviewData(
    const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities,
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

    // Add a row for each entity
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

        // Create row via inner RML
        std::string rowRml =
            "<tr>"
            "<td><span class=\"standing-dot " + standingClass + "\"></span></td>"
            "<td class=\"" + textClass + "\">" + entity->getShipName() + "</td>"
            "<td class=\"distance\">" + distStr + "</td>"
            "<td class=\"entity-type\">" + entityType + "</td>"
            "</tr>";

        body->SetInnerRML(body->GetInnerRML() + rowRml);
    }
}

void RmlUiManager::SetOverviewFilter(const std::string& filter) {
    (void)filter;
    // TODO: Filter overview table rows by standing
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
        {"ship_hud",  resourcePath_ + "/rml/ship_hud.rml",  true},
        {"overview",  resourcePath_ + "/rml/overview.rml",   true},
        {"fitting",   resourcePath_ + "/rml/fitting.rml",    false},
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

void RmlUiManager::SetupInputCallbacks() {
    if (!window_ || !context_) return;

    // Store context pointer for callbacks via GLFW user pointer
    // Note: this may conflict with other GLFW user pointer usage.
    // A more robust solution would use a callback registry.
    struct RmlCallbackData {
        Rml::Context* context;
        RenderInterface_GL3* renderer;
    };

    // We use lambdas with captures stored globally for simplicity
    static Rml::Context* s_context = context_;
    static RenderInterface_GL3* s_renderer = renderInterface_.get();
    static int s_active_mods = 0;

    glfwSetKeyCallback(window_, [](GLFWwindow*, int key, int, int action, int mods) {
        s_active_mods = mods;
        RmlGLFW::ProcessKeyCallback(s_context, key, action, mods);
    });

    glfwSetCharCallback(window_, [](GLFWwindow*, unsigned int codepoint) {
        RmlGLFW::ProcessCharCallback(s_context, codepoint);
    });

    glfwSetCursorPosCallback(window_, [](GLFWwindow* w, double xpos, double ypos) {
        RmlGLFW::ProcessCursorPosCallback(s_context, w, xpos, ypos, s_active_mods);
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow*, int button, int action, int mods) {
        s_active_mods = mods;
        RmlGLFW::ProcessMouseButtonCallback(s_context, button, action, mods);
    });

    glfwSetScrollCallback(window_, [](GLFWwindow*, double, double yoffset) {
        RmlGLFW::ProcessScrollCallback(s_context, yoffset, s_active_mods);
    });

    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow*, int width, int height) {
        s_renderer->SetViewport(width, height);
        RmlGLFW::ProcessFramebufferSizeCallback(s_context, width, height);
    });

    glfwSetWindowContentScaleCallback(window_, [](GLFWwindow*, float xscale, float) {
        RmlGLFW::ProcessContentScaleCallback(s_context, xscale);
    });

    glfwSetCursorEnterCallback(window_, [](GLFWwindow*, int entered) {
        RmlGLFW::ProcessCursorEnterCallback(s_context, entered);
    });
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

void RmlUiManager::SetShipStatus(const RmlShipData&) {}
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
    const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>&,
    const glm::vec3&) {}
void RmlUiManager::SetOverviewFilter(const std::string&) {}

void RmlUiManager::SetDocumentVisible(const std::string&, bool) {}
bool RmlUiManager::IsDocumentVisible(const std::string&) const { return false; }
void RmlUiManager::ToggleDocument(const std::string&) {}
void RmlUiManager::AddCombatLogMessage(const std::string&) {}

} // namespace UI

#endif // USE_RMLUI
