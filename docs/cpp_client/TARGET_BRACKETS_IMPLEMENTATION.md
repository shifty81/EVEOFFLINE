# Target Brackets & Combat Feedback Implementation Guide

## Overview

This document outlines the implementation plan for in-space target brackets and combat feedback notifications in the EVE OFFLINE C++ client.

## Phase 3: Target Brackets & Combat Feedback

### 1. In-Space Target Bracket Rendering

**Current State:**
- TargetCardInfo structure exists in `photon_widgets.h`
- Target cards are rendered in UI space (2D) via PhotonHUD
- Entity positions are tracked in 3D space

**Implementation Steps:**

#### 1.1 Add 3D-to-2D Screen Projection
```cpp
// In cpp_client/include/rendering/renderer.h
glm::vec2 worldToScreen(const glm::vec3& worldPos, 
                        const glm::mat4& viewProj,
                        int screenWidth, int screenHeight);
```

#### 1.2 Create Target Bracket Renderer
```cpp
// In cpp_client/include/rendering/target_bracket_renderer.h
class TargetBracketRenderer {
public:
    struct BracketInfo {
        glm::vec3 worldPos;
        float distance;
        std::string name;
        float shieldPct;
        float armorPct;
        float hullPct;
        bool isHostile;
        bool isLocked;
        bool isLocking;
    };
    
    void render(const std::vector<BracketInfo>& targets,
                const Camera& camera,
                int screenWidth, int screenHeight);
                
private:
    void drawCornerBrackets(const glm::vec2& screenPos, float size, const Color& color);
    void drawHealthBar(const glm::vec2& screenPos, const BracketInfo& info);
    void drawDistance(const glm::vec2& screenPos, float distance);
};
```

#### 1.3 Bracket Visual Design
Based on EVE Online's bracket system:
- **Corner brackets**: Four corners of a square around the target
- **Color coding**:
  - Red: Hostile/Criminal (-5.0 standing or lower)
  - Orange: Unlockable target
  - Yellow: Locking in progress
  - White: Locked and selected
  - Gray: Neutral
  - Blue: Friendly (+5.0 standing or higher)
- **Health indicators**: Small bars for shield/armor/hull below brackets
- **Distance text**: Above brackets in km format
- **Target name**: Below health bars

#### 1.4 Integration with EntityManager
```cpp
// In Application::render() or GameClient::update()
std::vector<TargetBracketRenderer::BracketInfo> brackets;
for (const auto& [id, entity] : entityManager.getAllEntities()) {
    if (entity->isTargetable()) {
        BracketInfo info;
        info.worldPos = entity->getPosition();
        info.distance = glm::distance(playerPos, entity->getPosition());
        info.name = entity->getShipName();
        info.shieldPct = entity->getHealth().currentShield / entity->getHealth().maxShield;
        info.armorPct = entity->getHealth().currentArmor / entity->getHealth().maxArmor;
        info.hullPct = entity->getHealth().currentHull / entity->getHealth().maxHull;
        info.isHostile = entity->isHostile();
        info.isLocked = targetingSystem.isLocked(id);
        brackets.push_back(info);
    }
}
targetBracketRenderer.render(brackets, camera, width, height);
```

### 2. Combat Damage Notifications

**Current State:**
- NotificationManager exists with `ShowCombatAlert()` method
- No combat event messages from server
- Entity health updates are received in STATE_UPDATE

**Implementation Steps:**

#### 2.1 Add Combat Event Messages to Protocol
```cpp
// In cpp_server/include/network/protocol_handler.h
enum class MessageType {
    // ... existing types ...
    COMBAT_DAMAGE,      // Damage dealt/received
    TARGET_LOCKED,      // Target lock acquired
    TARGET_LOST,        // Target lock lost
    WARP_SCRAMBLED,     // Warp disrupted
    CAPACITOR_EMPTY     // Cap empty alert
};
```

#### 2.2 Server-Side Combat Event Broadcasting
```cpp
// In cpp_server/src/systems/combat_system.cpp
void CombatSystem::processDamage(Entity* attacker, Entity* target, float damage) {
    // Apply damage...
    
    // Notify attacker's client
    if (attacker->hasComponent<Player>()) {
        std::string msg = protocol.createCombatDamageEvent(
            target->getId(),
            damage,
            damageType,
            "hit"  // "hit", "miss", "critical"
        );
        sendToPlayer(attacker->getPlayerId(), msg);
    }
    
    // Notify target's client
    if (target->hasComponent<Player>()) {
        std::string msg = protocol.createCombatDamageEvent(
            attacker->getId(),
            damage,
            damageType,
            "received"
        );
        sendToPlayer(target->getPlayerId(), msg);
    }
}
```

#### 2.3 Client-Side Combat Event Handling
```cpp
// In cpp_client/src/core/game_client.cpp
m_networkManager.registerHandler("combat_damage", [this](const std::string& data) {
    auto json = nlohmann::json::parse(data);
    std::string attackerId = json.value("attacker_id", "");
    float damage = json.value("damage", 0.0f);
    std::string damageType = json.value("damage_type", "");
    std::string eventType = json.value("event_type", "");
    
    if (eventType == "received") {
        // Show damage received notification
        std::string msg = "Took " + std::to_string(int(damage)) + " " + 
                         damageType + " damage";
        m_notificationManager->ShowCombatAlert(msg);
        
        // Play damage sound
        if (m_audioManager) {
            m_audioManager->playSFX("shield_hit");
        }
        
        // Trigger screen shake if significant damage
        if (damage > 100.0f) {
            m_camera.shake(0.05f, 0.3f);
        }
    } else if (eventType == "hit") {
        // Show damage dealt notification (less intrusive)
        std::string targetName = entityManager.getEntity(attackerId)->getShipName();
        std::string msg = std::to_string(int(damage)) + " to " + targetName;
        m_notificationManager->AddNotification("Hit", msg, 
            NotificationType::Combat, 2.0f);
    }
});
```

### 3. Targeting Lock/Unlock Visual Feedback

**Implementation Steps:**

#### 3.1 Add Target State Tracking
```cpp
// In cpp_client/include/core/targeting_system.h
enum class TargetState {
    NONE,
    LOCKING,     // Lock in progress
    LOCKED,      // Fully locked
    LOST         // Lock lost (target destroyed/out of range)
};

class ClientTargetingSystem {
public:
    void beginLock(const std::string& targetId, float lockTime);
    void updateLocking(float deltaTime);
    TargetState getTargetState(const std::string& targetId) const;
    
private:
    struct LockProgress {
        std::string targetId;
        float progress;      // 0.0 to 1.0
        float lockTime;      // Total time needed
    };
    std::vector<LockProgress> m_lockingTargets;
};
```

#### 3.2 Visual Feedback for Lock Progress
```cpp
// In target bracket renderer
void TargetBracketRenderer::drawCornerBrackets(const glm::vec2& screenPos, 
                                                float size, 
                                                const BracketInfo& info) {
    Color color;
    if (info.isLocked) {
        color = Color(1.0f, 1.0f, 1.0f, 1.0f);  // White - locked
    } else if (info.isLocking) {
        // Yellow with animated expansion
        color = Color(1.0f, 1.0f, 0.0f, 1.0f);
        float lockProgress = targetingSystem.getLockProgress(info.targetId);
        size *= (0.5f + 0.5f * lockProgress);  // Animate from 50% to 100%
    } else {
        color = Color(0.5f, 0.5f, 0.5f, 0.7f);  // Gray - unlocked
    }
    
    // Draw four corner brackets
    float cornerSize = 10.0f;
    // Top-left
    drawLine(screenPos + Vec2(-size, -size), 
             screenPos + Vec2(-size + cornerSize, -size), color);
    drawLine(screenPos + Vec2(-size, -size), 
             screenPos + Vec2(-size, -size + cornerSize), color);
    // ... (repeat for other 3 corners)
}
```

#### 3.3 Lock/Unlock Notifications
```cpp
// In network message handler
m_networkManager.registerHandler("target_locked", [this](const std::string& data) {
    auto json = nlohmann::json::parse(data);
    std::string targetId = json.value("target_id", "");
    auto entity = entityManager.getEntity(targetId);
    
    if (entity) {
        std::string msg = "Locked: " + entity->getShipName();
        m_notificationManager->ShowSuccess(msg);
        
        // Play lock sound
        if (m_audioManager) {
            m_audioManager->playSFX("target_locked");
        }
        
        // Update targeting state
        m_targetingSystem.setLocked(targetId);
    }
});

m_networkManager.registerHandler("target_lost", [this](const std::string& data) {
    auto json = nlohmann::json::parse(data);
    std::string targetId = json.value("target_id", "");
    std::string reason = json.value("reason", "lost");
    
    std::string msg = "Target lost";
    if (reason == "destroyed") msg = "Target destroyed";
    else if (reason == "out_of_range") msg = "Target out of range";
    
    m_notificationManager->ShowWarning(msg);
    m_targetingSystem.clearTarget(targetId);
});
```

## Testing Plan

### Unit Tests
1. Test worldToScreen projection with various camera angles
2. Test bracket color selection based on standing
3. Test lock progress calculation
4. Test notification filtering (don't spam on every hit)

### Integration Tests
1. Lock multiple targets and verify bracket states
2. Take damage and verify notifications appear
3. Deal damage and verify hit feedback
4. Verify brackets update when entities move
5. Test bracket occlusion (don't draw brackets behind camera)

### Visual Tests
1. Verify brackets are visible at all distances (scale appropriately)
2. Test bracket rendering with 50+ entities
3. Verify color coding matches EVE conventions
4. Test bracket readability against various backgrounds

## Performance Considerations

1. **Culling**: Only render brackets for entities within view frustum
2. **Distance culling**: Don't render brackets beyond max targeting range
3. **Batching**: Batch all bracket rendering into single draw call
4. **Text rendering**: Cache frequently used distance strings
5. **Update frequency**: Update bracket positions at 60 FPS but combat events at 10-20 Hz

## References

- EVE Online Photon UI Documentation
- `cpp_client/include/ui/photon/photon_widgets.h` - Existing target card UI
- `cpp_client/src/ui/notification_manager.cpp` - Notification system
- Server combat system: `cpp_server/src/systems/combat_system.cpp`

---

*Last Updated: February 11, 2026*
*Status: Design document - Implementation pending*
