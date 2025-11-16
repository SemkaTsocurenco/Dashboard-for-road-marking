#include "MarkingObjectListModel.h"
#include <cmath>

namespace viewmodels {

    MarkingObjectListModel::MarkingObjectListModel(QObject* parent)
        : QAbstractListModel(parent)
    {
    }

    void MarkingObjectListModel::updateFromDomain(const domain::MarkingObjectModel& model) {
        beginResetModel();

        objects_.clear();
        objects_.reserve(model.size());

        for (const auto& obj : model) {
            objects_.push_back(obj);
        }

        if (timestamp_ms_ != model.timestampMs()) {
            timestamp_ms_ = model.timestampMs();
            emit timestampChanged(timestamp_ms_);
        }

        endResetModel();
        emit countChanged(static_cast<int>(objects_.size()));
    }

    void MarkingObjectListModel::clear() {
        if (objects_.empty())
            return;

        beginResetModel();
        objects_.clear();
        timestamp_ms_ = 0;
        endResetModel();

        emit countChanged(0);
        emit timestampChanged(0);
    }

    int MarkingObjectListModel::rowCount(const QModelIndex& parent) const {
        if (parent.isValid())
            return 0;
        return static_cast<int>(objects_.size());
    }

    QVariant MarkingObjectListModel::data(const QModelIndex& index, int role) const {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(objects_.size()))
            return {};

        const auto& obj = objects_[static_cast<size_t>(index.row())];

        switch (role) {
            case ClassIdRole:
                return static_cast<int>(obj.classId());

            case ClassNameRole:
                return classIdToString(obj.classId());

            case XMetersRole:
                return obj.xMeters();

            case YMetersRole:
                return obj.yMeters();

            case LengthMetersRole:
                return obj.lengthMeters();

            case WidthMetersRole:
                return obj.widthMeters();

            case YawDegRole:
                return obj.yawDeg();

            case ConfidenceRole:
                return obj.confidence();

            case IsCrosswalkRole:
                return obj.isCrosswalk();

            case IsArrowRole:
                return obj.isArrow();

            case IsValidRole:
                return obj.isValid();

            case AreaRole:
                return obj.area();

            case DistanceRole: {
                // Calculate Euclidean distance from vehicle (0, 0)
                float x = obj.xMeters();
                float y = obj.yMeters();
                return std::sqrt(x * x + y * y);
            }

            default:
                return {};
        }
    }

    QHash<int, QByteArray> MarkingObjectListModel::roleNames() const {
        return {
            {ClassIdRole, "classId"},
            {ClassNameRole, "className"},
            {XMetersRole, "xMeters"},
            {YMetersRole, "yMeters"},
            {LengthMetersRole, "lengthMeters"},
            {WidthMetersRole, "widthMeters"},
            {YawDegRole, "yawDeg"},
            {ConfidenceRole, "confidence"},
            {IsCrosswalkRole, "isCrosswalk"},
            {IsArrowRole, "isArrow"},
            {IsValidRole, "isValid"},
            {AreaRole, "area"},
            {DistanceRole, "distance"}
        };
    }

    QString MarkingObjectListModel::classIdToString(laneproto::MarkingClassId id) const {
        switch (id) {
            case laneproto::MarkingClassId::Crosswalk:
                return QStringLiteral("Crosswalk");
            case laneproto::MarkingClassId::Arrow:
                return QStringLiteral("Arrow");
            case laneproto::MarkingClassId::Unknown:
            default:
                return QStringLiteral("Unknown");
        }
    }

}
