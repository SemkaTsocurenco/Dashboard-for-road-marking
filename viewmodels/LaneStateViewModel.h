#pragma once

#include <QObject>
#include <QVariantList>
#include <QPointF>
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
        Q_PROPERTY(bool hasDetails READ hasDetails NOTIFY detailsChanged)
        Q_PROPERTY(QString laneColorLeft READ laneColorLeft NOTIFY laneColorLeftChanged)
        Q_PROPERTY(QString laneColorRight READ laneColorRight NOTIFY laneColorRightChanged)
        Q_PROPERTY(float laneWidthLeftMeters READ laneWidthLeftMeters NOTIFY laneWidthLeftChanged)
        Q_PROPERTY(float laneWidthRightMeters READ laneWidthRightMeters NOTIFY laneWidthRightChanged)
        Q_PROPERTY(int laneQualityLeftPercent READ laneQualityLeftPercent NOTIFY laneQualityLeftChanged)
        Q_PROPERTY(int laneQualityRightPercent READ laneQualityRightPercent NOTIFY laneQualityRightChanged)
        Q_PROPERTY(QVariantList leftPoints READ leftPoints NOTIFY lanePointsChanged)
        Q_PROPERTY(QVariantList rightPoints READ rightPoints NOTIFY lanePointsChanged)

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
        bool hasDetails() const noexcept { return has_details_; }
        QString laneColorLeft() const { return lane_color_left_; }
        QString laneColorRight() const { return lane_color_right_; }
        float laneWidthLeftMeters() const noexcept { return lane_width_left_m_; }
        float laneWidthRightMeters() const noexcept { return lane_width_right_m_; }
        int laneQualityLeftPercent() const noexcept { return lane_quality_left_percent_; }
        int laneQualityRightPercent() const noexcept { return lane_quality_right_percent_; }
        QVariantList leftPoints() const { return left_points_; }
        QVariantList rightPoints() const { return right_points_; }

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
        void detailsChanged(bool hasDetails);
        void laneColorLeftChanged();
        void laneColorRightChanged();
        void laneWidthLeftChanged(float width);
        void laneWidthRightChanged(float width);
        void laneQualityLeftChanged(int quality);
        void laneQualityRightChanged(int quality);
        void lanePointsChanged();

    private:
        QString laneTypeToString(laneproto::LaneType type) const;
        QString lineColorToString(laneproto::LineColor color) const;
        QVariantList toVariantPoints(const std::array<laneproto::LanePoint, 3>& points,
                                     std::uint8_t count) const;

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
        bool has_details_{false};
        QString lane_color_left_{"Unknown"};
        QString lane_color_right_{"Unknown"};
        float lane_width_left_m_{0.0f};
        float lane_width_right_m_{0.0f};
        int lane_quality_left_percent_{0};
        int lane_quality_right_percent_{0};
        QVariantList left_points_;
        QVariantList right_points_;
    };

}
