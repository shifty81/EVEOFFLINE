#pragma once
#include <cstdint>
#include <functional>

namespace atlas::sim {

class TickScheduler {
public:
    void SetTickRate(uint32_t hz);
    uint32_t TickRate() const;

    float FixedDeltaTime() const;

    void Tick(const std::function<void(float)>& callback);
    uint64_t CurrentTick() const;

private:
    uint32_t m_tickRate = 30;
    uint64_t m_currentTick = 0;
};

}
