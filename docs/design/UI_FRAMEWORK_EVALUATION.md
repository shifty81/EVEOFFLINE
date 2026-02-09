# UI Framework Evaluation: Beyond ImGui

**Date**: February 9, 2026  
**Project**: EVE OFFLINE C++ Client  
**Purpose**: Evaluate UI framework alternatives to replace ImGui for Photon UI replication

---

## Why ImGui Is Insufficient

ImGui (Immediate Mode GUI) has served as the initial UI framework for EVE OFFLINE,
but it falls short for faithfully replicating EVE Online's Photon UI. The core
limitations are:

| Capability | ImGui | EVE Photon UI Needs |
|------------|-------|---------------------|
| **Layout model** | Immediate-mode, rectangular | Flexible CSS-like box model, circular elements |
| **Animations** | No built-in support | Smooth easing (150-500ms), pulsing, locking sequences |
| **Styling** | C++ color constants only | CSS-like stylesheets, themes, gradients, glow |
| **Custom shapes** | Manual PathArcTo hacks | Native arcs, rings, radial menus, polygons |
| **Rich text** | Single font, basic color | Multiple fonts, inline icons, markup |
| **Responsive scaling** | Manual per-widget | Percentage/flex layouts, breakpoints |
| **Drag-and-drop** | Basic payload system | Full DOM-like drag targets, ghost previews |
| **Accessibility** | Minimal | Colorblind modes, font scaling, high contrast |
| **Designer workflow** | C++ code only | Markup + stylesheet separation |

### Specific Pain Points

1. **Circular HUD gauges** require ~200 lines of manual `PathArcTo` + `PathStroke`
   per ring. Every visual tweak means recompiling C++.
2. **No animation system** — all transitions must be hand-coded with delta-time math.
3. **No stylesheet separation** — colors are hardcoded in `EVEColors` struct. Changing
   the theme requires editing C++ and rebuilding.
4. **Module rack, radial menu, and capacitor ring** all need custom OpenGL drawing
   that fights against ImGui's immediate-mode model.
5. **Window management** (snap, dock, minimize, opacity) must be manually implemented
   on top of ImGui's basic window system.

---

## Framework Comparison

### 1. RmlUi (Recommended)

**Repository**: https://github.com/mikke89/RmlUi  
**License**: MIT  
**Language**: C++17

| Aspect | Details |
|--------|---------|
| **Layout** | HTML/CSS box model (RML markup + RCSS stylesheets) |
| **Rendering** | Custom backend — integrates with any OpenGL/Vulkan renderer |
| **Animations** | CSS transitions, transforms, keyframe animations |
| **Styling** | RCSS (CSS2 + CSS3 subset), external stylesheets, theming |
| **Custom elements** | C++ custom element API for game-specific widgets |
| **Data binding** | Model-view data binding for live game state display |
| **Performance** | Lightweight, designed for real-time/game use |
| **Maturity** | Active development, used in multiple game engines (ezEngine, rbfx) |

**Why RmlUi is the best fit**:
- Markup-based layout means designers can iterate on the UI without C++ changes
- RCSS stylesheets can encode the entire Photon UI color palette and spacing
- Custom elements allow building circular gauges, radial menus as reusable components
- Data binding connects directly to game state (`ShipStatus`, `TargetInfo`, etc.)
- OpenGL backend integration is well-documented and proven
- MIT license is compatible with the project

**Integration effort**: Medium — requires implementing a render interface and
system interface, plus migrating panel layouts from C++ to RML/RCSS.

### 2. Ultralight

**Repository**: https://ultralig.ht  
**License**: Proprietary (free for non-commercial/indie)

| Aspect | Details |
|--------|---------|
| **Layout** | Full HTML5/CSS3 via embedded browser engine |
| **Rendering** | GPU-accelerated compositor, OpenGL/D3D backends |
| **Animations** | Full CSS animations, transitions, JavaScript |
| **Styling** | Complete CSS3 with web fonts, gradients, shadows |
| **Scripting** | JavaScript (V8/JavaScriptCore) |
| **Performance** | Higher memory (50-100MB), multi-process architecture |

**Pros**: Most powerful styling, full web standards, inspector tools.  
**Cons**: Proprietary license, heavy runtime (~50MB+), overkill for game HUD.  
**Verdict**: Too heavy and not fully open-source. Not recommended.

### 3. NoesisGUI

**Repository**: https://www.noesisengine.com  
**License**: Commercial (free for indie < $100k revenue)

| Aspect | Details |
|--------|---------|
| **Layout** | XAML (WPF-style) |
| **Rendering** | Custom GPU renderer, OpenGL/Vulkan/D3D |
| **Animations** | Full WPF animation system (storyboards, triggers) |
| **Styling** | XAML styles, templates, visual states |
| **Performance** | Excellent — designed for AAA games |

**Pros**: Most polished game UI framework, used in AAA titles.  
**Cons**: Commercial license, XAML is less intuitive than HTML/CSS.  
**Verdict**: Excellent but not open-source. Not recommended for this project.

### 4. Nuklear

**Repository**: https://github.com/Immediate-Mode-UI/Nuklear  
**License**: MIT/Public Domain

| Aspect | Details |
|--------|---------|
| **Layout** | Immediate-mode (like ImGui) |
| **Rendering** | Multiple backends |
| **Animations** | None |
| **Styling** | Skinning via style structs |

**Pros**: Simpler than ImGui, single-header.  
**Cons**: Same fundamental limitations as ImGui. Not an upgrade.  
**Verdict**: Lateral move. Not recommended.

### 5. Custom OpenGL UI Layer

| Aspect | Details |
|--------|---------|
| **Layout** | Custom flex/constraint system |
| **Rendering** | Direct OpenGL draw calls |
| **Animations** | Custom tween/animation system |
| **Styling** | JSON or custom config format |

**Pros**: Maximum control, no dependencies.  
**Cons**: Massive development effort (months), reinventing the wheel.  
**Verdict**: Only if all other options fail. Not recommended.

---

## Recommendation: Hybrid RmlUi + ImGui

**Primary UI**: Migrate to **RmlUi** for all game-facing panels (HUD, Overview,
Fitting, Market, Neocom, etc.).

**Debug/Dev UI**: Keep **ImGui** for developer tools, debug overlays, and
performance metrics.

### Migration Strategy

```
Phase 1: RmlUi Integration (Foundation)
  ├── Add RmlUi as dependency (CMake FetchContent or submodule)
  ├── Implement OpenGL 3.3 render interface
  ├── Implement GLFW system interface
  ├── Create Photon UI RCSS theme stylesheet
  └── Verify basic rendering alongside ImGui

Phase 2: Core Panel Migration
  ├── HUD (ship status rings, capacitor, speed) → RML + custom elements
  ├── Overview Panel → RML table with data binding
  ├── Target List → RML with custom circular elements
  └── Combat Log → RML scrolling list

Phase 3: Complex Panels
  ├── Fitting Window → RML with drag-and-drop
  ├── Inventory Panel → RML tree/grid layout
  ├── Market Panel → RML with charts and tables
  ├── Mission Panel → RML with rich text
  └── Neocom Sidebar → RML vertical icon bar

Phase 4: Advanced Features
  ├── Context Menu → RML popup
  ├── Radial Menu → RML + custom element
  ├── Window Management → RML window system
  ├── Layout Save/Load → Serialize RML state
  └── Accessibility (colorblind, scaling)
```

### Dual-Render Architecture

During migration, both frameworks coexist:

```cpp
// Main render loop
void Application::RenderUI() {
    // RmlUi renders game UI (HUD, panels, menus)
    rmlContext->Update();
    rmlContext->Render();
    
    // ImGui renders debug overlays (when enabled)
    if (showDebugUI) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        RenderDebugPanels();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
```

---

## RmlUi Key Concepts for EVE OFFLINE

### RML Documents (HTML-like)

```xml
<rml>
<head>
    <link type="text/rcss" href="photon_ui.rcss"/>
</head>
<body>
    <div id="ship-hud" class="hud-panel">
        <div class="capacitor-ring" data-value="{{ship.capacitor_pct}}"/>
        <div class="shield-ring" data-value="{{ship.shield_pct}}"/>
        <div class="speed-display">
            <span class="speed-value">{{ship.velocity}}</span>
            <span class="speed-unit">m/s</span>
        </div>
    </div>
</body>
</rml>
```

### RCSS Stylesheets (CSS-like)

```css
/* Photon UI Theme */
body {
    font-family: LatoLatin, sans-serif;
    color: #E6EDF3;
}

.hud-panel {
    background-color: #0D111780;
    border: 1px #283848;
    border-radius: 2dp;
    padding: 8dp;
}

.shield-ring {
    decorator: custom-ring;
    ring-color: #3399FF;
    ring-empty-color: #14304080;
    ring-thickness: 8dp;
}
```

### Custom Elements (C++)

```cpp
// Circular gauge element rendered in C++ via OpenGL
class CircularGaugeElement : public Rml::Element {
public:
    void OnRender() override {
        float value = GetAttribute<float>("data-value", 0.0f);
        // Custom OpenGL rendering for circular gauge
        DrawCircularGauge(GetAbsoluteOffset(), GetBox().GetSize(), value);
    }
};
```

---

## Cost/Benefit Summary

| Factor | ImGui (Current) | RmlUi (Proposed) |
|--------|-----------------|-------------------|
| **Visual fidelity** | 60% of Photon UI | 95% of Photon UI |
| **Development speed** | Fast for simple panels | Fast once templates exist |
| **Designer workflow** | C++ only | Markup + CSS (designer-friendly) |
| **Animation support** | Manual | CSS transitions + keyframes |
| **Theme changes** | Recompile | Edit RCSS file, hot-reload |
| **Custom widgets** | PathArcTo hacks | Custom element API |
| **Memory overhead** | ~2MB | ~5-10MB |
| **Learning curve** | Low (if you know C++) | Low (if you know HTML/CSS) |
| **Dependency weight** | Light | Medium |

**Conclusion**: RmlUi provides the best balance of visual capability, open-source
licensing, performance, and integration effort for replicating EVE Online's Photon
UI. ImGui should be retained only for debug/developer tools.

---

**Author**: EVE OFFLINE Development Team  
**Date**: February 9, 2026  
**Document Version**: 1.0
