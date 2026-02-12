#ifndef EVE_SYSTEMS_RESEARCH_SYSTEM_H
#define EVE_SYSTEMS_RESEARCH_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Research system for blueprint ME/TE research and invention
 *
 * Manages research jobs: ME research, TE research, and T2 invention.
 */
class ResearchSystem : public ecs::System {
public:
    explicit ResearchSystem(ecs::World* world);
    ~ResearchSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ResearchSystem"; }

    /**
     * @brief Start a Material Efficiency research job
     * @return job_id or empty string on failure
     */
    std::string startMEResearch(const std::string& lab_entity_id,
                                const std::string& owner_id,
                                const std::string& blueprint_id,
                                int target_level,
                                float total_time,
                                double install_cost);

    /**
     * @brief Start a Time Efficiency research job
     * @return job_id or empty string on failure
     */
    std::string startTEResearch(const std::string& lab_entity_id,
                                const std::string& owner_id,
                                const std::string& blueprint_id,
                                int target_level,
                                float total_time,
                                double install_cost);

    /**
     * @brief Start an invention job to create a T2 blueprint
     * @return job_id or empty string on failure
     */
    std::string startInvention(const std::string& lab_entity_id,
                               const std::string& owner_id,
                               const std::string& blueprint_id,
                               const std::string& output_blueprint_id,
                               const std::string& datacore_1,
                               const std::string& datacore_2,
                               float success_chance,
                               float total_time,
                               double install_cost);

    /**
     * @brief Get the number of active research jobs
     */
    int getActiveJobCount(const std::string& lab_entity_id);

    /**
     * @brief Get the number of completed research jobs
     */
    int getCompletedJobCount(const std::string& lab_entity_id);

    /**
     * @brief Get the number of failed invention jobs
     */
    int getFailedJobCount(const std::string& lab_entity_id);

private:
    int job_counter_ = 0;

    // Deterministic "random" for invention success
    // Uses a simple LCG to keep results predictable in tests
    unsigned int rng_state_ = 42;
    float nextRandom();
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_RESEARCH_SYSTEM_H
