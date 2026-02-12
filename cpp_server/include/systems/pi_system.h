#ifndef EVE_SYSTEMS_PI_SYSTEM_H
#define EVE_SYSTEMS_PI_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Planetary Interaction system
 *
 * Manages planetary colonies: extraction cycles, processing,
 * storage, and resource transfer to player inventory.
 */
class PISystem : public ecs::System {
public:
    explicit PISystem(ecs::World* world);
    ~PISystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "PISystem"; }

    /**
     * @brief Install a new extractor on a colony
     * @return true if successfully installed
     */
    bool installExtractor(const std::string& colony_entity_id,
                          const std::string& resource_type,
                          int quantity_per_cycle);

    /**
     * @brief Install a processor on a colony
     * @return true if successfully installed
     */
    bool installProcessor(const std::string& colony_entity_id,
                          const std::string& input_type,
                          const std::string& output_type,
                          int input_qty,
                          int output_qty);

    /**
     * @brief Get the quantity of a stored resource in a colony
     */
    int getStoredResource(const std::string& colony_entity_id,
                          const std::string& resource_type);

    /**
     * @brief Get the total number of stored resource units
     */
    int getTotalStored(const std::string& colony_entity_id);

    /**
     * @brief Get the number of extractors on a colony
     */
    int getExtractorCount(const std::string& colony_entity_id);

    /**
     * @brief Get the number of processors on a colony
     */
    int getProcessorCount(const std::string& colony_entity_id);

private:
    int extractor_counter_ = 0;
    int processor_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_PI_SYSTEM_H
