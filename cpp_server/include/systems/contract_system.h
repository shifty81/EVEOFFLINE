#ifndef EVE_SYSTEMS_CONTRACT_SYSTEM_H
#define EVE_SYSTEMS_CONTRACT_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace eve {
namespace systems {

class ContractSystem : public ecs::System {
public:
    explicit ContractSystem(ecs::World* world);
    ~ContractSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ContractSystem"; }

    bool createContract(const std::string& board_entity_id,
                        const std::string& issuer_id,
                        const std::string& type,
                        double isk_reward,
                        float duration_seconds);

    bool acceptContract(const std::string& board_entity_id,
                        const std::string& contract_id,
                        const std::string& acceptor_id);

    bool completeContract(const std::string& board_entity_id,
                          const std::string& contract_id);

    int getActiveContractCount(const std::string& board_entity_id);

    int getContractsByStatus(const std::string& board_entity_id,
                             const std::string& status);
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_CONTRACT_SYSTEM_H
