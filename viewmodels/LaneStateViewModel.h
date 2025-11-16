#pragma once

#include <QObject>
#include "LaneState.h"

namespace viewmodels {

    class LaneStateViewModel : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
        Q_PROPERTY(QString laneTypeLeft READ laneTypeLeft NOTIFY laneTypeLeftChanged)
        Q_PROPERTY(QString laneTypeRight READ laneTypeRight NOTIFY laneTypeRightChanged)
        Q_PROPERTY(float leftOffsetMeters READ leftOffsetMeters NOTIFY leftOffsetChanged)
        Q_PROPERTY(float rightOffsetMeters READ rightOffsetMeters NOTIFY rightOffsetChanged)
        Q_PROPERTY(float laneWidthMeters READ laneWidthMeters NOTIFY laneWidthChanged)
        Q_PROPERTY(float centerOffsetMeters READ centerOffsetMeters NOTIFY centerOffsetChanged)
        Q_PROPERTY(int qualityPercent READ qualityPercent NOTIFY qualityChanged)
        Q_PROPERTY(bool isQualityGood READ isQualityGood NOTIFY qualityChanged)
        Q_PROPERTY(quint64 timestampMs READ timestampMs NOTIFY timestampChanged)

    public:
        explicit LaneStateViewModel(QObject* parent = nullptr);
        ~LaneStateViewModel() override = default;

        void updateFromDomain(const domain::LaneState& state);
        void reset();

        bool isValid() const noexcept { return valid_; }
        QString laneTypeLeft() const;
        QString laneTypeRight() const;
        float leftOffsetMeters() const noexcept { return left_offset_m_; }
        float rightOffsetMeters() const noexcept { return right_offset_m_; }
        float laneWidthMeters() const noexcept { return lane_width_m_; }
        float centerOffsetMeters() const noexcept { return center_offset_m_; }
        int qualityPercent() const noexcept { return quality_percent_; }
        bool isQualityGood() const noexcept { return is_quality_good_; }
        quint64 timestampMs() const noexcept { return timestamp_ms_; }

    signals:
        void validChanged(bool valid);
        void laneTypeLeftChanged();
        void laneTypeRightChanged();
        void leftOffsetChanged(float offset);
        void rightOffsetChanged(float offset);
        void laneWidthChanged(float width);
        void centerOffsetChanged(float offset);
        void qualityChanged();
        void timestampChanged(quint64 timestamp);

    private:
        QString laneTypeToString(laneproto::LaneType type) const;

        bool valid_{false};
        laneproto::LaneType lane_type_left_{};
        laneproto::LaneType lane_type_right_{};
        float left_offset_m_{0.0f};
        float right_offset_m_{0.0f};
        float lane_width_m_{0.0f};
        float center_offset_m_{0.0f};
        int quality_percent_{0};
        bool is_quality_good_{false};
        quint64 timestamp_ms_{0};
    };

}
