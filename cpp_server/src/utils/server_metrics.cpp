#include "utils/server_metrics.h"
#include "utils/logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace atlas {
namespace utils {

ServerMetrics::ServerMetrics()
    : server_start_(std::chrono::steady_clock::now()),
      last_log_time_(std::chrono::steady_clock::now()) {}

void ServerMetrics::recordTickStart() {
    tick_start_ = std::chrono::steady_clock::now();
}

void ServerMetrics::recordTickEnd() {
    auto now = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(now - tick_start_).count();

    std::lock_guard<std::mutex> lock(mutex_);
    tick_sum_ms_ += ms;
    if (tick_count_window_ == 0) {
        tick_min_ms_ = ms;
        tick_max_ms_ = ms;
    } else {
        tick_max_ms_ = std::max(tick_max_ms_, ms);
        tick_min_ms_ = std::min(tick_min_ms_, ms);
    }
    ++tick_count_window_;
    ++tick_count_total_;
}

double ServerMetrics::getAvgTickMs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return (tick_count_window_ > 0) ? tick_sum_ms_ / tick_count_window_ : 0.0;
}

double ServerMetrics::getMaxTickMs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return (tick_count_window_ > 0) ? tick_max_ms_ : 0.0;
}

double ServerMetrics::getMinTickMs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return (tick_count_window_ > 0) ? tick_min_ms_ : 0.0;
}

uint64_t ServerMetrics::getTotalTicks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tick_count_total_;
}

void ServerMetrics::setEntityCount(int count) {
    std::lock_guard<std::mutex> lock(mutex_);
    entity_count_ = count;
}

void ServerMetrics::setPlayerCount(int count) {
    std::lock_guard<std::mutex> lock(mutex_);
    player_count_ = count;
}

int ServerMetrics::getEntityCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entity_count_;
}

int ServerMetrics::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return player_count_;
}

double ServerMetrics::getUptimeSeconds() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - server_start_).count();
}

std::string ServerMetrics::getUptimeString() const {
    int total = static_cast<int>(getUptimeSeconds());
    int days  = total / 86400;
    int hours = (total % 86400) / 3600;
    int mins  = (total % 3600) / 60;
    int secs  = total % 60;

    std::ostringstream oss;
    oss << days << "d " << hours << "h " << mins << "m " << secs << "s";
    return oss.str();
}

std::string ServerMetrics::summary() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "[Metrics] tick avg=";

    if (tick_count_window_ > 0) {
        oss << (tick_sum_ms_ / tick_count_window_) << "ms"
            << " min=" << tick_min_ms_ << "ms"
            << " max=" << tick_max_ms_ << "ms";
    } else {
        oss << "n/a";
    }

    oss << " | entities=" << entity_count_
        << " players=" << player_count_;

    // Uptime (compute without lock since server_start_ is immutable)
    {
        int total = static_cast<int>(
            std::chrono::duration<double>(
                std::chrono::steady_clock::now() - server_start_).count());
        int d = total / 86400;
        int h = (total % 86400) / 3600;
        int m = (total % 3600) / 60;
        int s = total % 60;
        oss << " | uptime " << d << "d " << h << "h " << m << "m " << s << "s";
    }

    oss << " | ticks=" << tick_count_total_;
    return oss.str();
}

void ServerMetrics::logSummaryIfDue(double interval_seconds) {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - last_log_time_).count();

    if (elapsed >= interval_seconds) {
        Logger::instance().info(summary());
        last_log_time_ = now;
        resetWindow();
    }
}

void ServerMetrics::resetWindow() {
    std::lock_guard<std::mutex> lock(mutex_);
    tick_sum_ms_ = 0.0;
    tick_max_ms_ = 0.0;
    tick_min_ms_ = 0.0;
    tick_count_window_ = 0;
}

} // namespace utils
} // namespace atlas
