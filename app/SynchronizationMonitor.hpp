#pragma once

#include <QObject>
#include <cstdint>

namespace app {

class SynchronizationMonitor : public QObject {
    Q_OBJECT
    Q_PROPERTY(int timestampDiffMs READ timestampDiffMs
               NOTIFY timestampDiffChanged)
    Q_PROPERTY(bool isSynchronized READ isSynchronized
               NOTIFY synchronizationChanged)
    Q_PROPERTY(quint64 lastDataTimestamp READ lastDataTimestamp
               NOTIFY dataTimestampChanged)
    Q_PROPERTY(quint64 lastVideoTimestamp READ lastVideoTimestamp
               NOTIFY videoTimestampChanged)
    Q_PROPERTY(int threshold READ threshold CONSTANT)

public:

    explicit SynchronizationMonitor(int threshold_ms, QObject* parent = nullptr);
    ~SynchronizationMonitor() override = default;

    void updateDataTimestamp(std::uint64_t timestamp_ms);
    void updateVideoTimestamp(std::uint64_t timestamp_ms);

    void reset();

    int timestampDiffMs() const { return timestamp_diff_ms_; }
    bool isSynchronized() const { return is_synchronized_; }
    quint64 lastDataTimestamp() const { return data_timestamp_ms_; }
    quint64 lastVideoTimestamp() const { return video_timestamp_ms_; }
    int threshold() const { return threshold_ms_; }

signals:

    void timestampDiffChanged(int diff_ms);
    void synchronizationChanged(bool synced);
    void dataTimestampChanged(quint64 timestamp);
    void videoTimestampChanged(quint64 timestamp);
    void desyncWarning(const QString& message);
    void syncRestored();

private:
    std::uint64_t data_timestamp_ms_{0};
    std::uint64_t video_timestamp_ms_{0};
    int timestamp_diff_ms_{0};
    int threshold_ms_;
    bool is_synchronized_{true};
    bool has_data_{false};
    bool has_video_{false};


    void checkSynchronization();
    void setTimestampDiff(int diff);
    void setSynchronized(bool synced);
};

} // namespace app
