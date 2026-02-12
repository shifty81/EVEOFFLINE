#ifndef EVE_SERVER_METRICS_H
#define EVE_SERVER_METRICS_H

#include <string>
#include <chrono>
#include <mutex>
#include <cstdint>

namespace atlas {
namespace utils {

/**
 * @brief Lightweight server performance metrics
 *
 * Tracks tick timing, entity/player counts, and uptime.
 * Call `recordTickStart()` and `recordTickEnd()` around the
 * main-loop body, and `logSummary()` periodically for a
 * human-readable status line.
 */
class ServerMetrics {
public:
    ServerMetrics();

    // --- Tick timing ---
    void recordTickStart();
    void recordTickEnd();

    /// Average tick duration (ms) over the last reporting window
    double getAvgTickMs() const;
    /// Worst-case tick duration (ms) over the last reporting window
    double getMaxTickMs() const;
    /// Best-case tick duration (ms) over the last reporting window
    double getMinTickMs() const;
    /// Total number of ticks recorded
    uint64_t getTotalTicks() const;

    // --- Counters ---
    void setEntityCount(int count);
    void setPlayerCount(int count);
    int  getEntityCount() const;
    int  getPlayerCount() const;

    // --- Uptime ---
    /// Seconds since the metrics object was created (server start)
    double getUptimeSeconds() const;

    /// Human-readable uptime string (e.g. "1d 3h 22m 15s")
    std::string getUptimeString() const;

    // --- Reporting ---
    /**
     * @brief Build a one-line status summary
     *
     * Example:
     *   "[Metrics] tick avg=2.13ms min=1.80ms max=4.21ms | entities=42 players=3 | uptime 0d 1h 5m 30s | ticks=113400"
     */
    std::string summary() const;

    /**
     * @brief Log the summary via the Logger singleton
     *
     * Only logs if at least `interval_seconds` have passed since
     * the last call (default: 60 s).
     */
    void logSummaryIfDue(double interval_seconds = 60.0);

    /// Reset tick-timing accumulators (keeps uptime & total tick count)
    void resetWindow();

private:
    // Tick timing – current window
    double tick_sum_ms_ = 0.0;
    double tick_max_ms_ = 0.0;
    double tick_min_ms_ = 0.0;
    uint64_t tick_count_window_ = 0;
    uint64_t tick_count_total_ = 0;

    std::chrono::steady_clock::time_point tick_start_;
    std::chrono::steady_clock::time_point server_start_;
    std::chrono::steady_clock::time_point last_log_time_;

    int entity_count_ = 0;
    int player_count_ = 0;

    mutable std::mutex mutex_;
};

} // namespace utils
} // namespace atlas

#endif // EVE_SERVER_METRICS_H
