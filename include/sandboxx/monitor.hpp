#ifndef SANDBOXX_MONITOR_HPP
#define SANDBOXX_MONITOR_HPP

#include "sandbox_config.hpp"
#include "cgroup_manager.hpp"
#include <thread>
#include <atomic>
#include <functional>
#include <vector>

namespace sandboxx {

struct MetricSnapshot {
    uint64_t timestamp_ms;
    double cpu_usage_pct;
    uint64_t memory_rss_bytes;
    uint32_t active_pids;
    uint64_t io_read_bytes;
    uint64_t io_write_bytes;
    bool throttled;
};

using MetricCallback = std::function<void(const MetricSnapshot&)>;

class Monitor {
public:
    Monitor(pid_t target_pid, CgroupManager* cgroup_mgr, uint32_t sample_interval_ms = 100);
    ~Monitor();

    bool start();
    void stop();
    
    std::vector<MetricSnapshot> get_history() const;
    MetricSnapshot get_latest_metrics() const;
    void register_callback(MetricCallback cb);

private:
    pid_t target_pid_;
    CgroupManager* cgroup_mgr_;
    uint32_t sample_interval_ms_;
    std::atomic<bool> is_running_{false};
    std::thread worker_thread_;

    mutable std::vector<MetricSnapshot> history_;
    std::vector<MetricCallback> callbacks_;

    void run_sampling_loop();
    MetricSnapshot sample_once();
};

} // namespace sandboxx

#endif // SANDBOXX_MONITOR_HPP
