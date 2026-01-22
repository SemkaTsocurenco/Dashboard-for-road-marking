#pragma once

#include <QAbstractListModel>
#include <QVariantList>
#include "FittedLine.h"

namespace viewmodels {

    class FittedLineListModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
        Q_PROPERTY(quint64 timestampMs READ timestampMs NOTIFY timestampChanged)
        Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)

    public:
        enum Roles {
            SideRole = Qt::UserRole + 1,
            SideNameRole,
            ColorRole,
            ColorNameRole,
            StyleRole,
            StyleNameRole,
            PolyARole,
            PolyBRole,
            PolyCRole,
            YStartRole,
            YEndRole,
            ConfidenceRole,
            QualityRole,
            IsValidRole,
            // V2 protocol fields - points in meters
            CenterXMetersRole,
            CenterYMetersRole,
            Point0XMetersRole,
            Point0YMetersRole,
            Point1XMetersRole,
            Point1YMetersRole,
            Point2XMetersRole,
            Point2YMetersRole,
            // Points list for QML
            PointsMetersRole
        };

        explicit FittedLineListModel(QObject* parent = nullptr);
        ~FittedLineListModel() override = default;

        void updateFromDomain(const domain::FittedLinesModel& model);
        void clear();

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

        quint64 timestampMs() const noexcept { return timestamp_ms_; }
        bool isValid() const noexcept { return valid_; }

    signals:
        void countChanged(int count);
        void timestampChanged(quint64 timestamp);
        void validChanged(bool valid);

    private:
        QString sideToString(laneproto::LineSide side) const;
        QString colorToString(laneproto::LineColor color) const;
        QString styleToString(laneproto::LineStyle style) const;

        std::vector<domain::FittedLine> lines_;
        quint64 timestamp_ms_{0};
        bool valid_{false};
    };

}
