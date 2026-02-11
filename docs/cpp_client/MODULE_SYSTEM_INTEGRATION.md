# Module System Integration Guide

## Overview

This document outlines the implementation plan for integrating the module system with the server, including UI clicks, state synchronization, and visual feedback in the EVE OFFLINE C++ client.

## Phase 4: Module System Integration

### Current State

**Client-Side:**
- Module slot UI widgets exist (`moduleSlot`, `moduleSlotEx` in `photon_widgets.h`)
- Support for active state, cooldown percentage, and overheat visualization
- Module rack rendered in PhotonHUD
- F1-F8 keyboard shortcuts partially implemented

**Server-Side:**
- ModuleSystem exists (`cpp_server/src/systems/module_system.cpp`)
- Module activation/deactivation protocol messages defined
- Module component with cooldown tracking

**Gap:**
- UI clicks not wired to network commands
- Module states not synchronized from server snapshots
- No cooldown timer updates from server
- Heat/overheat not tracked or visualized

## Implementation Plan

### 1. Module Activation from UI Clicks

#### 1.1 Wire Module Rack Clicks to Network Commands

```cpp
// In cpp_client/src/ui/photon/photon_hud.cpp
void PhotonHUD::drawModuleRack(PhotonContext& ctx, 
                               const std::vector<ModuleInfo>& modules,
                               NetworkManager& network) {
    float slotRadius = 24.0f;
    float slotSpacing = 60.0f;
    
    for (size_t i = 0; i < modules.size(); ++i) {
        Vec2 slotPos = calculateSlotPosition(i, slotSpacing);
        const auto& module = modules[i];
        
        // Determine slot color based on slot type
        Color slotColor = getSlotColor(module.slotType);
        
        // Draw module slot with current state
        bool clicked = moduleSlot(ctx, slotPos, slotRadius,
                                  module.active,
                                  module.cooldownPct,
                                  slotColor);
        
        if (clicked && !module.cooldownPct > 0.0f) {
            // Send activation command to server
            if (module.active) {
                network.sendModuleDeactivate(i);
            } else {
                network.sendModuleActivate(i);
            }
        }
        
        // Draw F-key label
        std::string label = "F" + std::to_string(i + 1);
        ctx.text(slotPos + Vec2(0, slotRadius + 12), label, Color::White);
    }
}
```

#### 1.2 Network Command Methods

```cpp
// In cpp_client/include/network/network_manager.h
class NetworkManager {
public:
    void sendModuleActivate(int slotIndex);
    void sendModuleDeactivate(int slotIndex);
    
private:
    std::string createModuleActivateMessage(int slot);
    std::string createModuleDeactivateMessage(int slot);
};

// In cpp_client/src/network/network_manager.cpp
void NetworkManager::sendModuleActivate(int slotIndex) {
    std::string msg = createModuleActivateMessage(slotIndex);
    send(msg);
    
    // Optional: Optimistically update local module state for instant feedback
    if (m_onModuleStateChanged) {
        m_onModuleStateChanged(slotIndex, true, 0.0f);
    }
}

std::string NetworkManager::createModuleActivateMessage(int slot) {
    nlohmann::json j;
    j["type"] = "module_activate";
    j["data"]["slot"] = slot;
    return j.dump();
}
```

#### 1.3 Keyboard Shortcut Handling

```cpp
// In cpp_client/src/core/application.cpp or game_client.cpp
void GameClient::handleInput(const InputState& input) {
    // F1-F8 module activation
    for (int i = 0; i < 8; ++i) {
        if (input.isKeyPressed(Key::F1 + i)) {
            if (m_modules.size() > i) {
                toggleModule(i);
            }
        }
    }
    
    // Ctrl+F1-F8 for overheating
    for (int i = 0; i < 8; ++i) {
        if (input.isKeyPressed(Key::F1 + i) && input.isModifierPressed(Modifier::Ctrl)) {
            if (m_modules.size() > i) {
                activateModuleWithOverheat(i);
            }
        }
    }
}

void GameClient::toggleModule(int slot) {
    if (m_modules[slot].active) {
        m_networkManager.sendModuleDeactivate(slot);
    } else {
        m_networkManager.sendModuleActivate(slot);
    }
}

void GameClient::activateModuleWithOverheat(int slot) {
    nlohmann::json j;
    j["type"] = "module_activate";
    j["data"]["slot"] = slot;
    j["data"]["overheat"] = true;
    m_networkManager.send(j.dump());
}
```

### 2. Module State Synchronization from Server

#### 2.1 Add Module State to Server Snapshots

```cpp
// In cpp_server/src/game_session.cpp - buildStateUpdate()
std::string GameSession::buildStateUpdate() const {
    // ... existing code ...
    
    // Add module states to player entities
    auto* player = entity->getComponent<components::Player>();
    if (player) {
        auto* modules = entity->getComponent<components::Modules>();
        if (modules) {
            json << ",\"modules\":[";
            bool firstModule = true;
            for (size_t i = 0; i < modules->slots.size(); ++i) {
                if (!firstModule) json << ",";
                firstModule = false;
                
                const auto& slot = modules->slots[i];
                json << "{"
                     << "\"active\":" << (slot.active ? "true" : "false")
                     << ",\"cooldown\":" << slot.cooldown
                     << ",\"cycle_time\":" << slot.cycle_time
                     << ",\"heat\":" << slot.heat
                     << ",\"module_id\":\"" << slot.module_id << "\""
                     << "}";
            }
            json << "]";
        }
    }
    
    // ... rest of entity state ...
}
```

#### 2.2 Parse Module State on Client

```cpp
// In cpp_client/src/core/entity_message_parser.cpp
bool EntityMessageParser::parseStateUpdate(const std::string& dataJson, 
                                          EntityManager& entityManager) {
    // ... existing parsing code ...
    
    // Extract module states for player entities
    if (entityData.contains("modules") && entityData["modules"].is_array()) {
        std::vector<ModuleState> modules;
        for (const auto& moduleJson : entityData["modules"]) {
            ModuleState state;
            state.active = moduleJson.value("active", false);
            state.cooldown = moduleJson.value("cooldown", 0.0f);
            state.cycleTime = moduleJson.value("cycle_time", 1.0f);
            state.heat = moduleJson.value("heat", 0.0f);
            state.moduleId = moduleJson.value("module_id", "");
            modules.push_back(state);
        }
        
        // Update entity module states
        entity->setModuleStates(modules);
    }
}
```

### 3. Module Cooldown Timer Visualization

#### 3.1 Calculate Cooldown Percentage

```cpp
// In Entity or ModuleManager
struct ModuleState {
    std::string moduleId;
    bool active;
    float cooldown;        // Remaining cooldown time in seconds
    float cycleTime;       // Full cycle time in seconds
    float heat;            // Current heat level 0.0-1.0
    
    float getCooldownPct() const {
        if (cycleTime <= 0.0f) return 0.0f;
        return std::clamp(cooldown / cycleTime, 0.0f, 1.0f);
    }
    
    float getHeatPct() const {
        return std::clamp(heat, 0.0f, 1.0f);
    }
};
```

#### 3.2 Update Cooldown Display Each Frame

```cpp
// In PhotonHUD::drawModuleRack()
for (size_t i = 0; i < modules.size(); ++i) {
    const auto& module = modules[i];
    
    // Calculate cooldown percentage for sweep overlay
    float cooldownPct = module.getCooldownPct();
    
    // Calculate heat percentage for overheat glow
    float heatPct = module.getHeatPct();
    
    // Draw module slot with extended overheat support
    bool clicked = moduleSlotEx(ctx, slotPos, slotRadius,
                                module.active,
                                cooldownPct,
                                slotColor,
                                heatPct,
                                gameTime);
    
    // Optional: Show numeric cooldown timer for long cycles
    if (cooldownPct > 0.0f && module.cooldown > 1.0f) {
        std::string timerText = std::to_string(int(module.cooldown + 0.5f)) + "s";
        ctx.text(slotPos, timerText, Color::White, TextAlign::Center);
    }
}
```

### 4. Heat & Overheat State Visualization

#### 4.1 Visual Heat Indicators

The `moduleSlotEx` widget already supports heat visualization. Implementation details:

```cpp
// In cpp_client/src/ui/photon/photon_widgets.cpp
bool moduleSlotEx(PhotonContext& ctx, Vec2 centre, float radius,
                  bool active, float cooldownPct, const Color& color,
                  float overheatPct, float time) {
    // Base module circle
    Color baseColor = color;
    if (!active) {
        baseColor.a *= 0.5f;  // Dim when inactive
    }
    
    ctx.circle(centre, radius, baseColor);
    
    // Heat glow effect
    if (overheatPct > 0.01f) {
        // Red/orange glow that increases with heat
        float glowRadius = radius + (4.0f * overheatPct);
        float glowAlpha = 0.3f * overheatPct;
        
        // Pulse animation when near burnout
        if (overheatPct > 0.8f) {
            float pulse = 0.5f + 0.5f * std::sin(time * 8.0f);
            glowAlpha *= pulse;
        }
        
        Color heatColor(1.0f, 0.3f, 0.0f, glowAlpha);  // Orange-red
        ctx.circle(centre, glowRadius, heatColor);
        
        // Draw heat bar below module
        float heatBarWidth = radius * 2.0f;
        float heatBarHeight = 3.0f;
        Vec2 barPos = centre + Vec2(-radius, radius + 8.0f);
        Rect heatBarBg = {barPos.x, barPos.y, heatBarWidth, heatBarHeight};
        Rect heatBarFill = {barPos.x, barPos.y, heatBarWidth * overheatPct, heatBarHeight};
        
        ctx.rect(heatBarBg, Color(0.2f, 0.2f, 0.2f, 0.5f));
        
        Color heatBarColor = overheatPct > 0.8f 
            ? Color(1.0f, 0.0f, 0.0f, 0.9f)   // Red when critical
            : Color(1.0f, 0.5f, 0.0f, 0.7f);  // Orange otherwise
        ctx.rect(heatBarFill, heatBarColor);
    }
    
    // Cooldown sweep overlay
    if (cooldownPct > 0.01f) {
        // Draw pie-slice sweep showing cooldown progress
        ctx.arc(centre, radius, 0.0f, cooldownPct * 360.0f, 
               Color(0.0f, 0.0f, 0.0f, 0.6f));
    }
    
    // Active pulse animation
    if (active && cooldownPct < 0.01f) {
        float pulse = 0.8f + 0.2f * std::sin(time * 4.0f);
        Color pulseColor = color;
        pulseColor.a = 0.4f * pulse;
        ctx.circle(centre, radius + 2.0f, pulseColor);
    }
    
    return ctx.buttonBehavior(Rect(centre.x - radius, centre.y - radius,
                                   radius * 2.0f, radius * 2.0f), 
                             WidgetID("module", int(centre.x)));
}
```

#### 4.2 Heat Damage Warnings

```cpp
// Monitor heat levels and show warnings
void GameClient::updateModuleHeat(float deltaTime) {
    for (size_t i = 0; i < m_modules.size(); ++i) {
        float heatPct = m_modules[i].getHeatPct();
        
        // Warn when approaching burnout
        if (heatPct > 0.95f && !m_heatWarningShown[i]) {
            std::string msg = "Module " + std::to_string(i + 1) + 
                             " critical heat!";
            m_notificationManager->ShowDanger(msg);
            m_heatWarningShown[i] = true;
            
            // Play warning sound
            if (m_audioManager) {
                m_audioManager->playSFX("module_overheat_warning");
            }
        } else if (heatPct < 0.9f) {
            m_heatWarningShown[i] = false;  // Reset warning flag
        }
        
        // Show burnout notification
        if (heatPct >= 1.0f && m_modules[i].wasBurntOut != true) {
            std::string msg = "Module " + std::to_string(i + 1) + 
                             " burnt out!";
            m_notificationManager->ShowDanger(msg);
            m_modules[i].wasBurntOut = true;
            
            // Play burnout sound
            if (m_audioManager) {
                m_audioManager->playSFX("module_burnout");
            }
        }
    }
}
```

### 5. Server-Side Module Cooldown & Heat Updates

#### 5.1 Module Cooldown Tracking

```cpp
// In cpp_server/src/systems/module_system.cpp
void ModuleSystem::update(float deltaTime) {
    for (auto* entity : world_->getEntitiesWithComponent<components::Modules>()) {
        auto* modules = entity->getComponent<components::Modules>();
        
        for (auto& slot : modules->slots) {
            if (slot.cooldown > 0.0f) {
                slot.cooldown -= deltaTime;
                if (slot.cooldown <= 0.0f) {
                    slot.cooldown = 0.0f;
                    // Module is ready to activate again
                }
            }
            
            // Update heat dissipation
            if (slot.heat > 0.0f) {
                // Heat dissipates over time (EVE: ~1% per second normally)
                float heatDissipation = 0.01f * deltaTime;
                slot.heat -= heatDissipation;
                slot.heat = std::max(0.0f, slot.heat);
            }
        }
    }
}
```

#### 5.2 Module Activation with Heat

```cpp
void ModuleSystem::activateModule(Entity* entity, int slot, bool overheat) {
    auto* modules = entity->getComponent<components::Modules>();
    if (!modules || slot < 0 || slot >= modules->slots.size()) {
        return;
    }
    
    auto& moduleSlot = modules->slots[slot];
    
    // Check if module is on cooldown
    if (moduleSlot.cooldown > 0.0f) {
        return;  // Can't activate while on cooldown
    }
    
    // Check if module is burnt out
    if (moduleSlot.heat >= 1.0f) {
        return;  // Module is burnt out, can't activate
    }
    
    // Activate the module
    moduleSlot.active = true;
    moduleSlot.cooldown = moduleSlot.cycle_time;
    
    // Apply heat if overheating
    if (overheat) {
        // Overheating adds 10-20% heat per cycle (varies by module)
        float heatGeneration = 0.15f;  // 15% heat per overheat cycle
        moduleSlot.heat += heatGeneration;
        moduleSlot.heat = std::min(1.0f, moduleSlot.heat);
        
        // Overheat bonus: -15% cycle time
        moduleSlot.cooldown *= 0.85f;
    }
    
    // Apply module effect (weapon fire, rep cycle, etc.)
    applyModuleEffect(entity, slot);
}
```

## Testing Plan

### Unit Tests

1. **Module Activation**:
   - Test F1-F8 keyboard shortcuts
   - Test mouse click on module slots
   - Test activation during cooldown (should be blocked)
   - Test activation when burnt out (should be blocked)

2. **Cooldown Calculation**:
   - Test cooldown percentage calculation
   - Test cooldown display updates each frame
   - Test cooldown expiry

3. **Heat Management**:
   - Test heat accumulation with overheat
   - Test heat dissipation over time
   - Test burnout at 100% heat
   - Test heat warning notifications

### Integration Tests

1. **Client-Server Sync**:
   - Activate module on client, verify server receives command
   - Server updates cooldown, verify client displays correct percentage
   - Test with network latency (100-200ms)
   - Test with packet loss (5-10%)

2. **Multi-Module Management**:
   - Activate all 8 modules simultaneously
   - Verify independent cooldown timers
   - Test heat accumulation across multiple modules

3. **Edge Cases**:
   - Disconnect while module is active
   - Module burns out during cycle
   - Change ship while modules are cycling

### Visual Tests

1. Module slot visual states:
   - Inactive (dim)
   - Active (bright + pulse)
   - Cooldown (sweep overlay)
   - Overheating (orange glow)
   - Critical heat (red glow + pulse)
   - Burnt out (red + crossed out)

2. Heat bar rendering:
   - Verify heat bar visibility
   - Test color transitions (orange → red)
   - Verify pulse animation near burnout

3. F-key labels:
   - Readable at all HUD scales
   - Correct alignment below slots

## Performance Considerations

1. **Update Frequency**:
   - Module state sync: 10-20 Hz (tied to server tick rate)
   - Visual updates: 60 FPS (interpolate between server updates)
   - Cooldown timers: Client-side prediction for responsiveness

2. **Network Optimization**:
   - Only send module state changes (delta compression)
   - Batch multiple module activations in single message
   - Client-side prediction for instant feedback

3. **Rendering**:
   - Batch all module slots into single draw call
   - Pre-calculate heat colors
   - Cache F-key label textures

## Audio Feedback

Module activation should include audio cues:

```cpp
void GameClient::onModuleActivated(int slot) {
    const auto& module = m_modules[slot];
    
    // Play activation sound based on module type
    if (module.type == ModuleType::Weapon) {
        m_audioManager->playSFX("weapon_fire");
    } else if (module.type == ModuleType::Shield) {
        m_audioManager->playSFX("shield_boost");
    } else if (module.type == ModuleType::Armor) {
        m_audioManager->playSFX("armor_rep");
    }
    
    // Play overheat sizzle if applicable
    if (module.heat > 0.8f) {
        m_audioManager->playSFX("module_overheat_sizzle");
    }
}
```

## References

- EVE Online module system mechanics
- `cpp_client/include/ui/photon/photon_widgets.h` - Module slot widgets
- `cpp_server/src/systems/module_system.cpp` - Server module system
- Server protocol: `cpp_server/include/network/protocol_handler.h`

---

*Last Updated: February 11, 2026*
*Status: Design document - Ready for implementation*
