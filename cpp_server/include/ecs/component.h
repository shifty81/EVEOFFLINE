#ifndef EVE_ECS_COMPONENT_H
#define EVE_ECS_COMPONENT_H

#include <string>
#include <memory>
#include <typeindex>

namespace atlas {
namespace ecs {

/**
 * @brief Base class for all components
 * 
 * Components are pure data containers that define entity properties.
 * They should not contain logic - that belongs in Systems.
 */
class Component {
public:
    virtual ~Component() = default;
    
    // Get type identifier for this component
    virtual std::type_index getTypeIndex() const = 0;
    
    // Clone this component (for copying entities)
    virtual std::unique_ptr<Component> clone() const = 0;
};

// Helper macro to define component type info
#define COMPONENT_TYPE(ClassName) \
    std::type_index getTypeIndex() const override { \
        return std::type_index(typeid(ClassName)); \
    } \
    std::unique_ptr<Component> clone() const override { \
        return std::make_unique<ClassName>(*this); \
    }

} // namespace ecs
} // namespace atlas

#endif // EVE_ECS_COMPONENT_H
