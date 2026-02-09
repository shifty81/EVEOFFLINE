/**
 * @file rml_ui_manager.cpp
 * @brief RmlUi-based UI manager implementation for EVE OFFLINE.
 *
 * This file provides the RmlUi integration layer:
 *  - OpenGL 3.3 render interface (vertex/index buffer submission)
 *  - GLFW system interface (time, logging, clipboard)
 *  - GLFW file interface (asset loading from ui_resources/)
 *  - Custom element registration for circular gauges
 *  - Data model setup for live game state binding
 *
 * The implementation is guarded by USE_RMLUI. When RmlUi is not available
 * the translation unit compiles to an empty stub so that the rest of the
 * client can still link.
 */

#ifdef USE_RMLUI

#include "ui/rml_ui_manager.h"

#include <RmlUi/Core.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace UI {

// ============================================================================
// RmlRenderInterface — OpenGL 3.3 backend for RmlUi
// ============================================================================

class RmlRenderInterface : public Rml::RenderInterface {
public:
    RmlRenderInterface() = default;
    ~RmlRenderInterface() override = default;

    void RenderGeometry(Rml::Vertex* vertices, int num_vertices,
                        int* indices, int num_indices,
                        Rml::TextureHandle texture,
                        const Rml::Vector2f& translation) override
    {
        glPushMatrix();
        glTranslatef(translation.x, translation.y, 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (texture) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
        } else {
            glDisable(GL_TEXTURE_2D);
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(2, GL_FLOAT, sizeof(Rml::Vertex),
                        &vertices[0].position);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rml::Vertex),
                       &vertices[0].colour);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Rml::Vertex),
                          &vertices[0].tex_coord);

        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        if (texture) {
            glDisable(GL_TEXTURE_2D);
        }

        glPopMatrix();
    }

    void EnableScissorRegion(bool enable) override {
        if (enable) {
            glEnable(GL_SCISSOR_TEST);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }
    }

    void SetScissorRegion(int x, int y, int width, int height) override {
        // RmlUi gives top-left origin; OpenGL scissor uses bottom-left
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glScissor(x, viewport[3] - (y + height), width, height);
    }

    bool LoadTexture(Rml::TextureHandle& texture_handle,
                     Rml::Vector2i& texture_dimensions,
                     const Rml::String& source) override
    {
        // Placeholder — integrate with existing Texture class for full support
        std::cout << "[RmlUI] LoadTexture: " << source << " (stub)\n";
        texture_handle = 0;
        texture_dimensions = {0, 0};
        return false;
    }

    bool GenerateTexture(Rml::TextureHandle& texture_handle,
                         const Rml::byte* source,
                         const Rml::Vector2i& source_dimensions) override
    {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     source_dimensions.x, source_dimensions.y,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, source);

        texture_handle = static_cast<Rml::TextureHandle>(tex);
        return true;
    }

    void ReleaseTexture(Rml::TextureHandle texture) override {
        GLuint tex = static_cast<GLuint>(texture);
        glDeleteTextures(1, &tex);
    }
};

// ============================================================================
// RmlSystemInterface — GLFW timing & logging
// ============================================================================

class RmlSystemInterface : public Rml::SystemInterface {
public:
    double GetElapsedTime() override {
        return glfwGetTime();
    }

    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override {
        const char* prefix = "[RmlUI]";
        switch (type) {
            case Rml::Log::LT_ERROR:   std::cerr << prefix << " ERROR: ";   break;
            case Rml::Log::LT_WARNING: std::cerr << prefix << " WARN: ";    break;
            case Rml::Log::LT_INFO:    std::cout << prefix << " INFO: ";    break;
            default:                   std::cout << prefix << " DEBUG: ";   break;
        }
        std::cout << message << "\n";
        return true;
    }
};

// ============================================================================
// RmlFileInterface — load assets from ui_resources/
// ============================================================================

class RmlFileInterface : public Rml::FileInterface {
public:
    explicit RmlFileInterface(const std::string& basePath)
        : basePath_(basePath) {}

    Rml::FileHandle Open(const Rml::String& path) override {
        std::string fullPath = basePath_ + "/" + path;
        auto* file = new std::ifstream(fullPath, std::ios::binary);
        if (!file->is_open()) {
            delete file;
            return 0;
        }
        return reinterpret_cast<Rml::FileHandle>(file);
    }

    void Close(Rml::FileHandle file) override {
        auto* stream = reinterpret_cast<std::ifstream*>(file);
        delete stream;
    }

    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override {
        auto* stream = reinterpret_cast<std::ifstream*>(file);
        stream->read(static_cast<char*>(buffer), static_cast<std::streamsize>(size));
        return static_cast<size_t>(stream->gcount());
    }

    bool Seek(Rml::FileHandle file, long offset, int origin) override {
        auto* stream = reinterpret_cast<std::ifstream*>(file);
        std::ios_base::seekdir dir;
        switch (origin) {
            case SEEK_SET: dir = std::ios::beg; break;
            case SEEK_CUR: dir = std::ios::cur; break;
            case SEEK_END: dir = std::ios::end; break;
            default: return false;
        }
        stream->seekg(offset, dir);
        return !stream->fail();
    }

    size_t Tell(Rml::FileHandle file) override {
        auto* stream = reinterpret_cast<std::ifstream*>(file);
        return static_cast<size_t>(stream->tellg());
    }

    size_t Length(Rml::FileHandle file) override {
        auto* stream = reinterpret_cast<std::ifstream*>(file);
        auto pos = stream->tellg();
        stream->seekg(0, std::ios::end);
        auto len = static_cast<size_t>(stream->tellg());
        stream->seekg(pos);
        return len;
    }

private:
    std::string basePath_;
};

// ============================================================================
// RmlUiManager implementation
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

    // Create interface implementations
    renderInterface_ = std::make_unique<RmlRenderInterface>();
    systemInterface_ = std::make_unique<RmlSystemInterface>();
    fileInterface_ = std::make_unique<RmlFileInterface>(resourcePath);

    // Set RmlUi interfaces
    Rml::SetRenderInterface(renderInterface_.get());
    Rml::SetSystemInterface(systemInterface_.get());
    Rml::SetFileInterface(fileInterface_.get());

    // Initialize RmlUi core
    if (!Rml::Initialise()) {
        std::cerr << "[RmlUiManager] Error: Rml::Initialise() failed\n";
        return false;
    }

    // Create context sized to the GLFW framebuffer
    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);

    context_ = Rml::CreateContext("main",
        Rml::Vector2i(fbWidth, fbHeight));
    if (!context_) {
        std::cerr << "[RmlUiManager] Error: failed to create RmlUi context\n";
        Rml::Shutdown();
        return false;
    }

    // Register custom elements (circular gauges, etc.)
    RegisterCustomElements();

    // Set up data model bindings
    SetupDataBindings();

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
        Rml::RemoveContext("main");
        context_ = nullptr;
    }

    Rml::Shutdown();

    renderInterface_.reset();
    systemInterface_.reset();
    fileInterface_.reset();

    initialized_ = false;
    std::cout << "[RmlUiManager] Shutdown complete\n";
}

// ----------------------------------------------------------------
// Per-frame
// ----------------------------------------------------------------

void RmlUiManager::ProcessInput() {
    if (!initialized_ || !context_) return;
    // Input forwarding is handled by GLFW callbacks registered
    // during Initialize(). Full implementation will map GLFW key,
    // mouse, and scroll events to context_->ProcessKey*,
    // context_->ProcessMouseMove, etc.
}

void RmlUiManager::Update() {
    if (!initialized_ || !context_) return;
    context_->Update();
}

void RmlUiManager::Render() {
    if (!initialized_ || !context_) return;

    // Set up 2D orthographic projection for UI rendering
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    context_->Render();

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ----------------------------------------------------------------
// Ship Status
// ----------------------------------------------------------------

void RmlUiManager::SetShieldPercent(float pct) {
    // Update via data model binding once fully integrated
    (void)pct;
}

void RmlUiManager::SetArmorPercent(float pct) {
    (void)pct;
}

void RmlUiManager::SetHullPercent(float pct) {
    (void)pct;
}

void RmlUiManager::SetCapacitorPercent(float pct) {
    (void)pct;
}

void RmlUiManager::SetVelocity(float velocity) {
    (void)velocity;
}

void RmlUiManager::SetMaxVelocity(float maxVelocity) {
    (void)maxVelocity;
}

void RmlUiManager::SetShipStatus(float shieldPct, float armorPct, float hullPct,
                                  float capPct, float velocity, float maxVelocity) {
    SetShieldPercent(shieldPct);
    SetArmorPercent(armorPct);
    SetHullPercent(hullPct);
    SetCapacitorPercent(capPct);
    SetVelocity(velocity);
    SetMaxVelocity(maxVelocity);
}

// ----------------------------------------------------------------
// Target List
// ----------------------------------------------------------------

void RmlUiManager::SetTarget(const std::string& id, const std::string& name,
                              float shieldPct, float armorPct, float hullPct,
                              float distance, bool isHostile, bool isActive) {
    (void)id; (void)name;
    (void)shieldPct; (void)armorPct; (void)hullPct;
    (void)distance; (void)isHostile; (void)isActive;
}

void RmlUiManager::RemoveTarget(const std::string& id) {
    (void)id;
}

void RmlUiManager::ClearTargets() {
    // Clear target list elements in the target document
}

// ----------------------------------------------------------------
// Overview
// ----------------------------------------------------------------

void RmlUiManager::UpdateOverviewData(
    const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities,
    const glm::vec3& playerPos)
{
    (void)entities; (void)playerPos;
    // Rebuild overview table rows from entity data
}

void RmlUiManager::SetOverviewFilter(const std::string& filter) {
    (void)filter;
}

// ----------------------------------------------------------------
// Panel Visibility
// ----------------------------------------------------------------

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

// ----------------------------------------------------------------
// Combat Log
// ----------------------------------------------------------------

void RmlUiManager::AddCombatLogMessage(const std::string& message) {
    (void)message;
    // Append a <div class="message"> to the combat log document
}

// ----------------------------------------------------------------
// Internal helpers
// ----------------------------------------------------------------

bool RmlUiManager::LoadDocuments() {
    if (!context_) return false;

    struct DocInfo {
        std::string name;
        std::string path;
        bool showByDefault;
    };

    std::vector<DocInfo> docs = {
        {"ship_hud",  "rml/ship_hud.rml",  true},
        {"overview",  "rml/overview.rml",   true},
        {"fitting",   "rml/fitting.rml",    false},
    };

    bool allOk = true;
    for (const auto& info : docs) {
        auto* doc = context_->LoadDocument(info.path);
        if (doc) {
            documents_[info.name] = doc;
            if (info.showByDefault) {
                doc->Show();
            }
            std::cout << "[RmlUiManager] Loaded document: " << info.name << "\n";
        } else {
            std::cerr << "[RmlUiManager] Failed to load: " << info.path << "\n";
            allOk = false;
        }
    }

    return allOk;
}

void RmlUiManager::RegisterCustomElements() {
    // Custom element instancers for circular gauges will be registered here.
    // Example:
    //   Rml::Factory::RegisterElementInstancer("shield-ring",
    //       new ShieldRingInstancer());
    //
    // Each custom element overrides OnRender() to draw the block-ring gauge
    // using OpenGL calls identical to the ImGui DrawBlockRingGauge() function
    // in eve_panels.cpp, but now driven by RML attribute data-value.
    std::cout << "[RmlUiManager] Custom elements registered (stubs)\n";
}

void RmlUiManager::SetupDataBindings() {
    // Data model bindings allow RML templates to reference live game state
    // via {{ship.shield_pct}}, {{ship.velocity}}, etc.
    //
    // Full implementation:
    //   auto constructor = context_->CreateDataModel("ship");
    //   constructor.Bind("shield_pct", &shipModel_.shieldPct);
    //   constructor.Bind("velocity",   &shipModel_.velocity);
    //   ...
    //   shipModelHandle_ = constructor.GetModelHandle();
    std::cout << "[RmlUiManager] Data bindings set up (stubs)\n";
}

} // namespace UI

#else // !USE_RMLUI

// ============================================================================
// Stub implementation when RmlUi is not available
// ============================================================================

#include "ui/rml_ui_manager.h"
#include <iostream>

namespace UI {

// Provide complete type definitions so unique_ptr destructors compile
class RmlRenderInterface {};
class RmlSystemInterface {};
class RmlFileInterface {};

RmlUiManager::RmlUiManager() = default;
RmlUiManager::~RmlUiManager() = default;

bool RmlUiManager::Initialize(GLFWwindow*, const std::string&) {
    std::cout << "[RmlUiManager] RmlUi not enabled (build with -DUSE_RMLUI=ON)\n";
    return false;
}

void RmlUiManager::Shutdown() {}
void RmlUiManager::ProcessInput() {}
void RmlUiManager::Update() {}
void RmlUiManager::Render() {}

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

bool RmlUiManager::LoadDocuments() { return false; }
void RmlUiManager::RegisterCustomElements() {}
void RmlUiManager::SetupDataBindings() {}

} // namespace UI

#endif // USE_RMLUI
