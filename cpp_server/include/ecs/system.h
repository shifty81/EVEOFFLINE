#ifndef EVE_ECS_SYSTEM_H
#define EVE_ECS_SYSTEM_H

#include <string>

namespace atlas {
namespace ecs {

// Forward declaration
class World;

/**
 * @brief Base class for all systems
 * 
 * Systems operate on entities with specific component combinations.
 * They contain the game logic that processes component data.
 */
class System {
public:
    explicit System(World* world) : world_(world) {}
    virtual ~System() = default;
    
    /**
     * @brief Update this system
     * @param delta_time Time elapsed since last update (in seconds)
     */
    virtual void update(float delta_time) = 0;
    
    /**
     * @brief Get system name for debugging
     */
    virtual std::string getName() const = 0;
    
protected:
    World* world_;
};

} // namespace ecs
} // namespace atlas

#endif // EVE_ECS_SYSTEM_H
