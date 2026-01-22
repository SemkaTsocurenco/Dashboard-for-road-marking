#include "FittedLineListModel.h"
#include "LoggerMacros.hpp"

namespace viewmodels {

    FittedLineListModel::FittedLineListModel(QObject* parent)
        : QAbstractListModel(parent)
    {
    }

    void FittedLineListModel::updateFromDomain(const domain::FittedLinesModel& model) {
        beginResetModel();

        lines_.clear();
        lines_.reserve(model.size());

        for (const auto& line : model.lines()) {
            LOG_DEBUG << "FittedLine from domain: side=" << static_cast<int>(line.side())
                     << " color=" << static_cast<int>(line.color())
                     << " style=" << static_cast<int>(line.style())
                     << " poly=[" << line.polyA() << "," << line.polyB() << "," << line.polyC() << "]"
                     << " valid=" << line.isValid();
            lines_.push_back(line);
        }

        bool wasValid = valid_;
        valid_ = model.isValid();

        if (timestamp_ms_ != model.timestampMs()) {
            timestamp_ms_ = model.timestampMs();
            emit timestampChanged(timestamp_ms_);
        }

        if (wasValid != valid_) {
            emit validChanged(valid_);
        }

        endResetModel();
        emit countChanged(static_cast<int>(lines_.size()));
    }

    void FittedLineListModel::clear() {
        if (lines_.empty() && !valid_)
            return;

        beginResetModel();
        lines_.clear();
        timestamp_ms_ = 0;
        valid_ = false;
        endResetModel();

        emit countChanged(0);
        emit timestampChanged(0);
        emit validChanged(false);
    }

    int FittedLineListModel::rowCount(const QModelIndex& parent) const {
        if (parent.isValid())
            return 0;
        return static_cast<int>(lines_.size());
    }

    QVariant FittedLineListModel::data(const QModelIndex& index, int role) const {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(lines_.size()))
            return {};

        const auto& line = lines_[static_cast<size_t>(index.row())];

        switch (role) {
            case SideRole:
                return static_cast<int>(line.side());

            case SideNameRole:
                return sideToString(line.side());

            case ColorRole:
                return static_cast<int>(line.color());

            case ColorNameRole:
                return colorToString(line.color());

            case StyleRole:
                return static_cast<int>(line.style());

            case StyleNameRole:
                return styleToString(line.style());

            case PolyARole:
                return line.polyA();

            case PolyBRole:
                return line.polyB();

            case PolyCRole:
                return line.polyC();

            case YStartRole:
                return line.yStart();

            case YEndRole:
                return line.yEnd();

            case ConfidenceRole:
                return line.confidence();

            case QualityRole:
                return line.quality();

            case IsValidRole:
                return line.isValid();

            case CenterXMetersRole:
                return line.centerXMeters();

            case CenterYMetersRole:
                return line.centerYMeters();

            case Point0XMetersRole:
                return line.pointsMeters()[0].x_m;

            case Point0YMetersRole:
                return line.pointsMeters()[0].y_m;

            case Point1XMetersRole:
                return line.pointsMeters()[1].x_m;

            case Point1YMetersRole:
                return line.pointsMeters()[1].y_m;

            case Point2XMetersRole:
                return line.pointsMeters()[2].x_m;

            case Point2YMetersRole:
                return line.pointsMeters()[2].y_m;

            case PointsMetersRole: {
                QVariantList points;
                for (const auto& pt : line.pointsMeters()) {
                    QVariantMap point;
                    point["x"] = pt.x_m;
                    point["y"] = pt.y_m;
                    points.append(point);
                }
                return points;
            }

            default:
                return {};
        }
    }

    QHash<int, QByteArray> FittedLineListModel::roleNames() const {
        return {
            {SideRole, "side"},
            {SideNameRole, "sideName"},
            {ColorRole, "lineColor"},
            {ColorNameRole, "lineColorName"},
            {StyleRole, "lineStyle"},
            {StyleNameRole, "lineStyleName"},
            {PolyARole, "polyA"},
            {PolyBRole, "polyB"},
            {PolyCRole, "polyC"},
            {YStartRole, "yStart"},
            {YEndRole, "yEnd"},
            {ConfidenceRole, "confidence"},
            {QualityRole, "quality"},
            {IsValidRole, "isValid"},
            {CenterXMetersRole, "centerXMeters"},
            {CenterYMetersRole, "centerYMeters"},
            {Point0XMetersRole, "point0XMeters"},
            {Point0YMetersRole, "point0YMeters"},
            {Point1XMetersRole, "point1XMeters"},
            {Point1YMetersRole, "point1YMeters"},
            {Point2XMetersRole, "point2XMeters"},
            {Point2YMetersRole, "point2YMeters"},
            {PointsMetersRole, "pointsMeters"}
        };
    }

    QString FittedLineListModel::sideToString(laneproto::LineSide side) const {
        switch (side) {
            case laneproto::LineSide::Left:
                return QStringLiteral("Left");
            case laneproto::LineSide::Right:
                return QStringLiteral("Right");
            case laneproto::LineSide::Center:
                return QStringLiteral("Center");
            case laneproto::LineSide::Unknown:
            default:
                return QStringLiteral("Unknown");
        }
    }

    QString FittedLineListModel::colorToString(laneproto::LineColor color) const {
        switch (color) {
            case laneproto::LineColor::White:
                return QStringLiteral("White");
            case laneproto::LineColor::Yellow:
                return QStringLiteral("Yellow");
            case laneproto::LineColor::Red:
                return QStringLiteral("Red");
            case laneproto::LineColor::Unknown:
            default:
                return QStringLiteral("Unknown");
        }
    }

    QString FittedLineListModel::styleToString(laneproto::LineStyle style) const {
        switch (style) {
            case laneproto::LineStyle::Solid:
                return QStringLiteral("Solid");
            case laneproto::LineStyle::Dashed:
                return QStringLiteral("Dashed");
            case laneproto::LineStyle::Double:
                return QStringLiteral("Double");
            case laneproto::LineStyle::Unknown:
            default:
                return QStringLiteral("Unknown");
        }
    }

}
