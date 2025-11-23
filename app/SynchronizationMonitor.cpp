#include "SynchronizationMonitor.hpp"
#include "LoggerMacros.hpp"
#include <cmath>

namespace app {

SynchronizationMonitor::SynchronizationMonitor(int threshold_ms, QObject* parent)
    : QObject(parent)
    , threshold_ms_(threshold_ms)
{
    LOG_DEBUG << "SynchronizationMonitor created with threshold=" << threshold_ms << "ms";
}

void SynchronizationMonitor::updateDataTimestamp(std::uint64_t timestamp_ms) {
    data_timestamp_ms_ = timestamp_ms;

    if (!has_data_) {
        has_data_ = true;
        LOG_DEBUG << "SynchronizationMonitor: First data timestamp received: " << timestamp_ms;
    }

    emit dataTimestampChanged(timestamp_ms);
    checkSynchronization();
}

void SynchronizationMonitor::updateVideoTimestamp(std::uint64_t timestamp_ms) {
    video_timestamp_ms_ = timestamp_ms;

    if (!has_video_) {
        has_video_ = true;
        LOG_DEBUG << "SynchronizationMonitor: First video timestamp received: " << timestamp_ms;
    }

    emit videoTimestampChanged(timestamp_ms);
    checkSynchronization();
}

void SynchronizationMonitor::reset() {
    LOG_INFO << "SynchronizationMonitor: Resetting state";

    data_timestamp_ms_ = 0;
    video_timestamp_ms_ = 0;
    has_data_ = false;
    has_video_ = false;

    setTimestampDiff(0);
    setSynchronized(true);
}

void SynchronizationMonitor::checkSynchronization() {
    if (!has_data_ || !has_video_) {
        return;
    }

    std::uint64_t diff;
    if (data_timestamp_ms_ > video_timestamp_ms_) {
        diff = data_timestamp_ms_ - video_timestamp_ms_;
    } else {
        diff = video_timestamp_ms_ - data_timestamp_ms_;
    }

    int diff_ms = static_cast<int>(diff);

    setTimestampDiff(diff_ms);

    bool currently_synced = (diff_ms <= threshold_ms_);

    if (currently_synced != is_synchronized_) {
        if (!currently_synced) {
            QString msg = QString("Desynchronization detected: %1ms (threshold: %2ms). "
                                "Data: %3ms, Video: %4ms")
                .arg(diff_ms)
                .arg(threshold_ms_)
                .arg(data_timestamp_ms_)
                .arg(video_timestamp_ms_);

            LOG_WARN << msg.toStdString();
            emit desyncWarning(msg);
        } else {
            LOG_INFO << "Synchronization restored: diff=" << diff_ms << "ms";
            emit syncRestored();
        }

        setSynchronized(currently_synced);
    }
}

void SynchronizationMonitor::setTimestampDiff(int diff) {
    if (timestamp_diff_ms_ != diff) {
        timestamp_diff_ms_ = diff;
        emit timestampDiffChanged(diff);
    }
}

void SynchronizationMonitor::setSynchronized(bool synced) {
    if (is_synchronized_ != synced) {
        is_synchronized_ = synced;
        emit synchronizationChanged(synced);

        LOG_DEBUG << "Synchronization status changed: "
                  << (synced ? "SYNCHRONIZED" : "DESYNCHRONIZED");
    }
}

} // namespace app
