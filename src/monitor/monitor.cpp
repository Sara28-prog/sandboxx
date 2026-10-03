#include "sandboxx/monitor.hpp"
#include "sandboxx/logger.hpp"
#include "sandboxx/utils.hpp"
#include <chrono>
#include <fstream>

namespace sandboxx {

Monitor::Monitor(pid_t target_pid, CgroupManager* cgroup_mgr, uint32_t sample_interval_ms)
    : target_pid_(target_pid), cgroup_mgr_(cgroup_mgr), sample_interval_ms_(sample_interval_ms) {}

Monitor::~Monitor() {
    stop();
}

bool Monitor::start() {
    is_running_ = true;
    worker_thread_ = std::thread(&Monitor::run_sampling_loop, this);
    LOG_INFO("Telemetry monitor thread started for PID " + std::to_string(target_pid_));
    return true;
}

void Monitor::stop() {
    if (is_running_) {
        is_running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        LOG_INFO("Telemetry monitor thread stopped");
    }
}

void Monitor::run_sampling_loop() {
    while (is_running_) {
        MetricSnapshot snapshot = sample_once();
        history_.push_back(snapshot);

        for (auto& cb : callbacks_) {
            cb(snapshot);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(sample_interval_ms_));
    }
}

MetricSnapshot Monitor::sample_once() {
    MetricSnapshot snapshot;
    snapshot.timestamp_ms = utils::current_time_millis();
    snapshot.cpu_usage_pct = 0.0;
    snapshot.memory_rss_bytes = 0;
    snapshot.active_pids = 1;
    snapshot.io_read_bytes = 0;
    snapshot.io_write_bytes = 0;
    snapshot.throttled = false;

    if (cgroup_mgr_) {
        CgroupStats cg_stats = cgroup_mgr_->read_stats();
        snapshot.memory_rss_bytes = cg_stats.memory_current_bytes;
        snapshot.active_pids = cg_stats.current_pids;
        snapshot.throttled = cg_stats.nr_throttled > 0;
    }

    // Also read /proc/<pid>/stat if cgroup gave zero
    std::string stat_path = "/proc/" + std::to_string(target_pid_) + "/stat";
    std::string stat_data;
    if (utils::read_file(stat_path, stat_data)) {
        // Parse fields
        auto tokens = utils::split(stat_data, ' ');
        if (tokens.size() > 23) {
            try {
                // RSS pages in token 23
                uint64_t rss_pages = std::stoull(tokens[23]);
                if (snapshot.memory_rss_bytes == 0) {
                    snapshot.memory_rss_bytes = rss_pages * 4096;
                }
            } catch (...) {}
        }
    }

    return snapshot;
}

std::vector<MetricSnapshot> Monitor::get_history() const {
    return history_;
}

MetricSnapshot Monitor::get_latest_metrics() const {
    if (history_.empty()) {
        MetricSnapshot empty{};
        return empty;
    }
    return history_.back();
}

void Monitor::register_callback(MetricCallback cb) {
    callbacks_.push_back(cb);
}

} // namespace sandboxx
