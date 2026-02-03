#include "ecs/entity.h"

namespace eve {
namespace ecs {

Entity::Entity(const std::string& id) : id_(id) {
}

bool Entity::hasComponents(const std::vector<std::type_index>& types) const {
    for (const auto& type : types) {
        if (components_.find(type) == components_.end()) {
            return false;
        }
    }
    return true;
}

} // namespace ecs
} // namespace eve
