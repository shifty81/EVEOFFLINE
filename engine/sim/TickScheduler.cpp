#include "TickScheduler.h"

namespace atlas::sim {

void TickScheduler::SetTickRate(uint32_t hz) {
    m_tickRate = hz > 0 ? hz : 1;
}

uint32_t TickScheduler::TickRate() const {
    return m_tickRate;
}

float TickScheduler::FixedDeltaTime() const {
    return 1.0f / static_cast<float>(m_tickRate);
}

void TickScheduler::Tick(const std::function<void(float)>& callback) {
    if (callback) {
        callback(FixedDeltaTime());
    }
    m_currentTick++;
}

uint64_t TickScheduler::CurrentTick() const {
    return m_currentTick;
}

}
