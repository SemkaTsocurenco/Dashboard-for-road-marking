#pragma once

#include <QAbstractListModel>
#include "Warning.h"

namespace viewmodels {

    class WarningListModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
        Q_PROPERTY(int activeCount READ activeCount NOTIFY activeCountChanged)
        Q_PROPERTY(int criticalCount READ criticalCount NOTIFY criticalCountChanged)
        Q_PROPERTY(bool hasCritical READ hasCritical NOTIFY hasCriticalChanged)
        Q_PROPERTY(quint64 lastUpdateMs READ lastUpdateMs NOTIFY lastUpdateChanged)

    public:
        enum Roles {
            TypeRole = Qt::UserRole + 1,
            TypeNameRole,
            SeverityRole,
            SeverityNameRole,
            TimestampMsRole,
            DistanceMetersRole,
            ConfidenceRole,
            MessageRole,
            IsActiveRole,
            IsCriticalRole
        };

        explicit WarningListModel(QObject* parent = nullptr);
        ~WarningListModel() override = default;

        void updateFromDomain(const domain::WarningModel& model);
        void clear();

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

        int activeCount() const noexcept { return active_count_; }
        int criticalCount() const noexcept { return critical_count_; }
        bool hasCritical() const noexcept { return critical_count_ > 0; }
        quint64 lastUpdateMs() const noexcept { return last_update_ms_; }

    signals:
        void countChanged(int count);
        void activeCountChanged(int count);
        void criticalCountChanged(int count);
        void hasCriticalChanged(bool hasCritical);
        void lastUpdateChanged(quint64 timestamp);

    private:
        QString warningTypeToString(domain::WarningType type) const;
        QString warningSeverityToString(domain::WarningSeverity severity) const;
        void updateCounters();

        std::vector<domain::Warning> warnings_;
        quint64 last_update_ms_{0};
        int active_count_{0};
        int critical_count_{0};
    };

}
