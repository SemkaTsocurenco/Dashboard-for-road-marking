#include "WarningListModel.h"

namespace viewmodels {

    WarningListModel::WarningListModel(QObject* parent)
        : QAbstractListModel(parent)
    {
    }

    void WarningListModel::updateFromDomain(const domain::WarningModel& model) {
        beginResetModel();

        warnings_.clear();
        warnings_.reserve(model.size());

        for (const auto& warning : model) {
            warnings_.push_back(warning);
        }

        if (last_update_ms_ != model.lastUpdateMs()) {
            last_update_ms_ = model.lastUpdateMs();
            emit lastUpdateChanged(last_update_ms_);
        }

        endResetModel();
        emit countChanged(static_cast<int>(warnings_.size()));

        updateCounters();
    }

    void WarningListModel::clear() {
        if (warnings_.empty())
            return;

        beginResetModel();
        warnings_.clear();
        last_update_ms_ = 0;
        endResetModel();

        emit countChanged(0);
        emit lastUpdateChanged(0);

        if (active_count_ != 0) {
            active_count_ = 0;
            emit activeCountChanged(0);
        }

        if (critical_count_ != 0) {
            critical_count_ = 0;
            emit criticalCountChanged(0);
            emit hasCriticalChanged(false);
        }
    }

    int WarningListModel::rowCount(const QModelIndex& parent) const {
        if (parent.isValid())
            return 0;
        return static_cast<int>(warnings_.size());
    }

    QVariant WarningListModel::data(const QModelIndex& index, int role) const {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(warnings_.size()))
            return {};

        const auto& warning = warnings_[static_cast<size_t>(index.row())];

        switch (role) {
            case TypeRole:
                return static_cast<int>(warning.type());

            case TypeNameRole:
                return warningTypeToString(warning.type());

            case SeverityRole:
                return static_cast<int>(warning.severity());

            case SeverityNameRole:
                return warningSeverityToString(warning.severity());

            case TimestampMsRole:
                return static_cast<qulonglong>(warning.timestampMs());

            case DistanceMetersRole:
                return warning.distanceMeters();

            case ConfidenceRole:
                return warning.confidence();

            case MessageRole:
                return QString::fromStdString(warning.message());

            case IsActiveRole:
                return warning.isActive();

            case IsCriticalRole:
                return warning.isCritical();

            default:
                return {};
        }
    }

    QHash<int, QByteArray> WarningListModel::roleNames() const {
        return {
            {TypeRole, "warningType"},
            {TypeNameRole, "typeName"},
            {SeverityRole, "severity"},
            {SeverityNameRole, "severityName"},
            {TimestampMsRole, "timestampMs"},
            {DistanceMetersRole, "distanceMeters"},
            {ConfidenceRole, "confidence"},
            {MessageRole, "message"},
            {IsActiveRole, "isActive"},
            {IsCriticalRole, "isCritical"}
        };
    }

    QString WarningListModel::warningTypeToString(domain::WarningType type) const {
        switch (type) {
            case domain::WarningType::CrosswalkAhead:
                return QStringLiteral("CrosswalkAhead");
            case domain::WarningType::LaneDepartureLeft:
                return QStringLiteral("LaneDepartureLeft");
            case domain::WarningType::LaneDepartureRight:
                return QStringLiteral("LaneDepartureRight");
            case domain::WarningType::SolidLineCross:
                return QStringLiteral("SolidLineCross");
            case domain::WarningType::Custom:
                return QStringLiteral("Custom");
            case domain::WarningType::Unknown:
            default:
                return QStringLiteral("Unknown");
        }
    }

    QString WarningListModel::warningSeverityToString(domain::WarningSeverity severity) const {
        switch (severity) {
            case domain::WarningSeverity::Info:
                return QStringLiteral("Info");
            case domain::WarningSeverity::Warning:
                return QStringLiteral("Warning");
            case domain::WarningSeverity::Critical:
                return QStringLiteral("Critical");
            default:
                return QStringLiteral("Unknown");
        }
    }

    void WarningListModel::updateCounters() {
        int new_active_count = 0;
        int new_critical_count = 0;

        for (const auto& warning : warnings_) {
            if (warning.isActive()) {
                ++new_active_count;
            }
            if (warning.isCritical()) {
                ++new_critical_count;
            }
        }

        if (active_count_ != new_active_count) {
            active_count_ = new_active_count;
            emit activeCountChanged(active_count_);
        }

        bool old_has_critical = critical_count_ > 0;
        bool new_has_critical = new_critical_count > 0;

        if (critical_count_ != new_critical_count) {
            critical_count_ = new_critical_count;
            emit criticalCountChanged(critical_count_);
        }

        if (old_has_critical != new_has_critical) {
            emit hasCriticalChanged(new_has_critical);
        }
    }

}
