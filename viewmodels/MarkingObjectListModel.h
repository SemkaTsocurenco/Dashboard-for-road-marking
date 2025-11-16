#pragma once

#include <QAbstractListModel>
#include "MarkingObject.h"

namespace viewmodels {

    class MarkingObjectListModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
        Q_PROPERTY(quint64 timestampMs READ timestampMs NOTIFY timestampChanged)

    public:
        enum Roles {
            ClassIdRole = Qt::UserRole + 1,
            ClassNameRole,
            XMetersRole,
            YMetersRole,
            LengthMetersRole,
            WidthMetersRole,
            YawDegRole,
            ConfidenceRole,
            IsCrosswalkRole,
            IsArrowRole,
            IsValidRole,
            AreaRole,
            DistanceRole  // Distance from vehicle (calculated from x, y)
        };

        explicit MarkingObjectListModel(QObject* parent = nullptr);
        ~MarkingObjectListModel() override = default;

        void updateFromDomain(const domain::MarkingObjectModel& model);
        void clear();

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

        quint64 timestampMs() const noexcept { return timestamp_ms_; }

    signals:
        void countChanged(int count);
        void timestampChanged(quint64 timestamp);

    private:
        QString classIdToString(laneproto::MarkingClassId id) const;

        std::vector<domain::MarkingObject> objects_;
        quint64 timestamp_ms_{0};
    };

}
