#include "ConnectionManager.h"
#include "FittedLineListModel.h"
#include "LoggerMacros.hpp"
#include "proto_parser.h"
#include "AppConfig.hpp"
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <QTimer>
#include <cmath>

namespace {
    laneproto::LaneType laneTypeFromStyle(laneproto::LineStyle style) {
        switch (style) {
            case laneproto::LineStyle::Solid:
                return laneproto::LaneType::Solid;
            case laneproto::LineStyle::Dashed:
                return laneproto::LaneType::Dashed;
            case laneproto::LineStyle::Double:
                return laneproto::LaneType::DoubleSolid;
            case laneproto::LineStyle::Unknown:
            default:
                return laneproto::LaneType::Unknown;
        }
    }

    float estimateOffsetFromPoints(const std::array<laneproto::PointMeters, 3>& points) {
        float best_x = points[0].x_m;
        float best_y = std::fabs(points[0].y_m);
        for (const auto& p : points) {
            const float abs_y = std::fabs(p.y_m);
            if (abs_y < best_y) {
                best_y = abs_y;
                best_x = p.x_m;
            }
        }
        return best_x;
    }

    void fillBoundaryDetails(laneproto::LaneBoundaryDetails& dst, const laneproto::LaneLine& line) {
        dst.type = laneTypeFromStyle(line.style);
        dst.color = line.color;
        dst.quality = 255;
        dst.width_m = 0.1f;
        dst.points_count = static_cast<std::uint8_t>(line.points_m.size());
        for (std::size_t i = 0; i < line.points_m.size(); ++i) {
            dst.points[i].x_m = line.points_m[i].x_m;
            dst.points[i].y_m = line.points_m[i].y_m;
        }
    }
}

namespace network {
    ConnectionManager::ConnectionManager(QObject* parent)
        : QObject(parent)
        , reconnect_timer_(new QTimer(this))
        , lane_view_model_(new viewmodels::LaneStateViewModel(this))
        , marking_list_model_(new viewmodels::MarkingObjectListModel(this))
        , warning_list_model_(new viewmodels::WarningListModel(this))
        , fitted_line_list_model_(new viewmodels::FittedLineListModel(this))
    {
        reconnect_timer_->setSingleShot(true);
        connect(reconnect_timer_, &QTimer::timeout, this, &ConnectionManager::attemptReconnect);
    }

    ConnectionManager::~ConnectionManager(){
        destroyWorker();
    }

    void ConnectionManager::setWarningEngineConfig(const domain::WarningEngineConfig& config) {
        warning_engine_.setConfig(config);
        LOG_INFO << "WarningEngine configuration updated: "
                 << "lane_departure_threshold=" << config.lane_departure_offset_threshold_m << "m, "
                 << "crosswalk_threshold=" << config.crosswalk_distance_threshold_m << "m";
    }

    void ConnectionManager::connectToHost(const QString& host, int port) {
        // Validate input parameters
        if (host.isEmpty()) {
            LOG_ERROR << "Cannot connect: empty host";
            setLastError("Invalid host: empty string");
            setState(State::Error);
            return;
        }

        if (port <= 0 || port > config::NetworkConfig::MAX_PORT) {
            LOG_ERROR << "Cannot connect: invalid port " << port;
            setLastError(QString("Invalid port: must be between 1 and %1").arg(config::NetworkConfig::MAX_PORT));
            setState(State::Error);
            return;
        }

        if (state_ == State::Connected ||
            state_ == State::Connecting ||
            state_ == State::Reconnecting) {
            LOG_WARN << "Already connected or connecting, ignoring request";
            return;
        }

        reconnect_timer_->stop();
        resetReconnectState();

        saved_host_ = host;
        saved_port_ = static_cast<quint16>(port);

        setLastError(QString{});
        setState(State::Connecting);
        createWorkerIfNeeded();

        QMetaObject::invokeMethod(
            worker_,
            "start",
            Qt::QueuedConnection,
            Q_ARG(QString, host),
            Q_ARG(quint16, static_cast<quint16>(port)));
    }

    void ConnectionManager::disconnectFromHost() {
        if (state_ == State::Disconnected || state_ == State::Disconnecting) {
            return;
        }

        reconnect_timer_->stop();
        resetReconnectState();

        setState(State::Disconnecting);
        if (worker_){
            QMetaObject::invokeMethod(worker_, "stop", Qt::QueuedConnection);
        } else {
            setState(State::Disconnected);
        }
    }

    bool ConnectionManager::isConnected() const {
        return connected_;
    }

    ConnectionManager::State ConnectionManager::state() const {
        return state_;
    }

    QString ConnectionManager::lastError() const {
        return last_error_;
    }

    void ConnectionManager::setState(State newState) {
        if (state_ == newState)
            return;
        state_ = newState;

        emit stateChanged(state_);

        bool nowConnected = (state_ == State::Connected);
        setConnected(nowConnected);

        if (state_ == State::Connected) {
            resetReconnectState();
        }
    }

    void ConnectionManager::setConnected(bool connected) {
        if (connected_ == connected)
            return;
        connected_ = connected;

        emit connectedChanged(connected_);
    }

    void ConnectionManager::setLastError(const QString& message) {
        if (last_error_ == message) 
            return;

        last_error_ = message;
        emit lastErrorChanged(last_error_);
    }

    void ConnectionManager::createWorkerIfNeeded() {
        if (worker_) return;

        workerThread_ = new QThread(this);
        worker_ = new TcpReaderWorker();

        worker_->moveToThread(workerThread_);

        connect(workerThread_, &QThread::finished,
                worker_, &QObject::deleteLater);
        connect(worker_, &TcpReaderWorker::connected, this, [this](){
            setState(State::Connected);
        });
        connect(worker_, &TcpReaderWorker::disconnected, this, [this]() {
            if (state_ == State::Disconnecting) {
                setState(State::Disconnected);
            } else if (state_ == State::Connected || state_ == State::Connecting) {
                setState(State::Disconnected);
                scheduleReconnect();
            }
        });
        connect(worker_, &TcpReaderWorker::errorOccurred, this, [this](const QString& message) {
            setLastError(message);
            setState(State::Error);
            scheduleReconnect();
        });
        // V2 Protocol signal connections
        connect(worker_, &TcpReaderWorker::laneLinesParsed,
                this, &ConnectionManager::laneLinesReceivedSlot);
        connect(worker_, &TcpReaderWorker::roadObjectsParsed,
                this, &ConnectionManager::roadObjectsReceivedSlot);

        // Legacy V1 signal connections (not used with V2 protocol)
        connect(worker_, &TcpReaderWorker::laneSummaryParsed,
                this, &ConnectionManager::laneSummaryReceived);
        connect(worker_, &TcpReaderWorker::markingObjectsParsed,
                this, &ConnectionManager::markingObjectsReceived);
        connect(worker_, &TcpReaderWorker::laneDetailsParsed,
                this, &ConnectionManager::laneDetailsReceived);
        connect(worker_, &TcpReaderWorker::markingObjectsExParsed,
                this, &ConnectionManager::markingObjectsExReceived);
        connect(worker_, &TcpReaderWorker::fittedLinesParsed,
                this, &ConnectionManager::fittedLinesReceived);
        connect(worker_, &TcpReaderWorker::parseErrorOccurred,
                this, &ConnectionManager::parseErrorReceived);

        workerThread_->start();
    }

    void ConnectionManager::destroyWorker(){
        if (!workerThread_)
            return;

        workerThread_->quit();
        workerThread_->wait();
        workerThread_->deleteLater();
        workerThread_ = nullptr;
        worker_ = nullptr;
    }

    void ConnectionManager::scheduleReconnect() {
        if (!auto_reconnect_) {
            LOG_DEBUG << "Auto-reconnect disabled, skipping reconnect";
            return;
        }

        if (saved_host_.isEmpty()) {
            LOG_WARN << "Cannot reconnect: no saved host";
            return;
        }

        if (max_reconnect_attempts_ > 0 && current_reconnect_attempt_ >= max_reconnect_attempts_) {
            LOG_ERROR << "Max reconnect attempts (" << max_reconnect_attempts_ << ") reached";
            setLastError("Max reconnect attempts reached");
            return;
        }

        current_reconnect_attempt_++;
        LOG_INFO << "Scheduling reconnect attempt " << current_reconnect_attempt_
                 << (max_reconnect_attempts_ > 0 ? " of " + std::to_string(max_reconnect_attempts_) : " (unlimited)")
                 << " in " << reconnect_interval_ << "ms";

        emit reconnectAttempt(current_reconnect_attempt_, max_reconnect_attempts_);

        setState(State::Reconnecting);
        reconnect_timer_->start(reconnect_interval_);
    }

    void ConnectionManager::attemptReconnect() {
        if (saved_host_.isEmpty()) {
            LOG_WARN << "Reconnect attempt aborted: no saved host";
            return;
        }

        if (state_ != State::Reconnecting) {
            LOG_WARN << "Reconnect attempt aborted: invalid state";
            return;
        }

        LOG_INFO << "Attempting to reconnect to " << saved_host_.toStdString() << ":" << saved_port_;

        setState(State::Connecting);
        createWorkerIfNeeded();

        QMetaObject::invokeMethod(
            worker_,
            "start",
            Qt::QueuedConnection,
            Q_ARG(QString, saved_host_),
            Q_ARG(quint16, saved_port_));
    }

    void ConnectionManager::resetReconnectState() {
        if (current_reconnect_attempt_ > 0) {
            LOG_INFO << "Resetting reconnect state (was at attempt " << current_reconnect_attempt_ << ")";
        }
        current_reconnect_attempt_ = 0;
    }

    void ConnectionManager::setAutoReconnect(bool enabled) {
        if (auto_reconnect_ == enabled)
            return;
        auto_reconnect_ = enabled;

        if (!enabled) {
            reconnect_timer_->stop();
            resetReconnectState();
        }
    }

    bool ConnectionManager::autoReconnect() const {
        return auto_reconnect_;
    }

    void ConnectionManager::setReconnectInterval(int milliseconds) {
        if (reconnect_interval_ == milliseconds)
            return;

        if (milliseconds < config::NetworkConfig::MIN_RECONNECT_INTERVAL_MS ||
            milliseconds > config::NetworkConfig::MAX_RECONNECT_INTERVAL_MS) {
            LOG_WARN << "Invalid reconnect interval: " << milliseconds
                     << " (must be " << config::NetworkConfig::MIN_RECONNECT_INTERVAL_MS
                     << "-" << config::NetworkConfig::MAX_RECONNECT_INTERVAL_MS << "ms), ignoring";
            return;
        }

        reconnect_interval_ = milliseconds;
        LOG_DEBUG << "Reconnect interval set to " << milliseconds << "ms";
    }

    int ConnectionManager::reconnectInterval() const {
        return reconnect_interval_;
    }

    void ConnectionManager::setMaxReconnectAttempts(int attempts) {
        if (max_reconnect_attempts_ == attempts)
            return;

        // Validate max reconnect attempts (0 = unlimited, max limit from config)
        if (attempts < 0 || attempts > config::NetworkConfig::MAX_RECONNECT_ATTEMPTS_LIMIT) {
            LOG_WARN << "Invalid max reconnect attempts: " << attempts
                     << " (must be 0-" << config::NetworkConfig::MAX_RECONNECT_ATTEMPTS_LIMIT << "), ignoring";
            return;
        }

        max_reconnect_attempts_ = attempts;
        LOG_DEBUG << "Max reconnect attempts set to "
                  << (attempts == 0 ? "unlimited" : std::to_string(attempts));
    }

    int ConnectionManager::maxReconnectAttempts() const {
        return max_reconnect_attempts_;
    }

    int ConnectionManager::currentReconnectAttempt() const {
        return current_reconnect_attempt_;
    }

    void ConnectionManager::updateWarnings(const std::uint64_t timestamp_ms) {
        auto warnings = warning_engine_.update(lane_state_, marking_model_, timestamp_ms);
        warning_model_.clear();
        warning_model_.reserve(warnings.size());
        for (auto& w : warnings) {
            warning_model_.addWarning(std::move(w));
        }
        warning_model_.setLastUpdateMs(timestamp_ms);
        LOG_DEBUG << "WarningModel updated: " << warning_model_;

        // Update ViewModel
        warning_list_model_->updateFromDomain(warning_model_);

        emit warningModelUpdated();
    }


    void ConnectionManager::laneSummaryReceived(const laneproto::LaneSummary& summary){
        lane_state_.updateFromProto(summary);
        LOG_DEBUG << "LaneState updated: " << lane_state_;

        // Update ViewModel
        lane_view_model_->updateFromDomain(lane_state_);

        emit laneStateUpdated();
        const std::uint64_t timestamp_ms = lane_state_.timestampMs();
        updateWarnings(timestamp_ms);
    }

    void ConnectionManager::laneDetailsReceived(const laneproto::LaneDetails& details){
        lane_state_.updateFromProto(details);
        LOG_DEBUG << "LaneState (details) updated: " << lane_state_;

        lane_view_model_->updateFromDomain(lane_state_);

        emit laneStateUpdated();
        const std::uint64_t timestamp_ms = lane_state_.timestampMs();
        updateWarnings(timestamp_ms);
    }

    void ConnectionManager::markingObjectsReceived(const laneproto::MarkingObjects& objects){
        marking_model_.updateFromProto(objects);
        LOG_DEBUG << "MarkingObjectModel updated: " << marking_model_;

        // Update ViewModel
        marking_list_model_->updateFromDomain(marking_model_);

        emit markingModelUpdated();
        if (lane_state_.isValid()) {
            const std::uint64_t timestamp_ms = marking_model_.timestampMs();
            updateWarnings(timestamp_ms);
        }
    }

    void ConnectionManager::markingObjectsExReceived(const laneproto::MarkingObjects& objects){
        marking_model_.updateFromProto(objects);
        LOG_DEBUG << "MarkingObjectModel (extended) updated: " << marking_model_;

        marking_list_model_->updateFromDomain(marking_model_);

        emit markingModelUpdated();
        if (lane_state_.isValid()) {
            const std::uint64_t timestamp_ms = marking_model_.timestampMs();
            updateWarnings(timestamp_ms);
        }
    }

    void ConnectionManager::fittedLinesReceived(const laneproto::FittedLines& lines){
        fitted_lines_model_.updateFromProto(lines);
        LOG_DEBUG << "FittedLinesModel updated: " << fitted_lines_model_;

        // Update ViewModel for QML
        fitted_line_list_model_->updateFromDomain(fitted_lines_model_);

        emit fittedLinesModelUpdated();
    }

    // V2 Protocol slot implementations
    void ConnectionManager::laneLinesReceivedSlot(const laneproto::LaneLines& lines) {
        LOG_DEBUG << "LaneLines (V2) received: lines=" << lines.lines.size()
                  << ", timestamp=" << lines.timestamp_ms;

        // Re-emit for external consumers
        emit laneLinesReceived(lines);

        // Update fitted lines model from V2 data
        fitted_lines_model_.updateFromProtoV2(lines);
        // Update ViewModel for QML
        fitted_line_list_model_->updateFromDomain(fitted_lines_model_);
        emit fittedLinesModelUpdated();

        if (lines.lines.empty()) {
            lane_state_.reset();
            lane_view_model_->updateFromDomain(lane_state_);
            emit laneStateUpdated();
            updateWarnings(lines.timestamp_ms);
            return;
        }

        const laneproto::LaneLine* left_line = nullptr;
        const laneproto::LaneLine* right_line = nullptr;
        for (const auto& line : lines.lines) {
            if (line.side == laneproto::LineSide::Left && !left_line) {
                left_line = &line;
            } else if (line.side == laneproto::LineSide::Right && !right_line) {
                right_line = &line;
            }

            if (left_line && right_line) {
                break;
            }
        }

        laneproto::LaneSummary summary;
        summary.timestamp_ms = lines.timestamp_ms;
        summary.seq = lines.seq;
        summary.lane_type_left = left_line ? laneTypeFromStyle(left_line->style) : laneproto::LaneType::Unknown;
        summary.lane_type_right = right_line ? laneTypeFromStyle(right_line->style) : laneproto::LaneType::Unknown;
        summary.allowed_maneuvers = 0xFF;
        summary.quality = 255;
        if (left_line && right_line) {
            summary.left_offset_m = estimateOffsetFromPoints(left_line->points_m);
            summary.right_offset_m = estimateOffsetFromPoints(right_line->points_m);
        } else {
            summary.left_offset_m = 0.0f;
            summary.right_offset_m = 0.0f;
        }

        lane_state_.updateFromProto(summary);

        laneproto::LaneDetails details{};
        details.timestamp_ms = lines.timestamp_ms;
        details.seq = lines.seq;
        details.left_offset_m = summary.left_offset_m;
        details.right_offset_m = summary.right_offset_m;
        details.allowed_maneuvers = summary.allowed_maneuvers;
        details.quality = summary.quality;
        if (left_line) {
            fillBoundaryDetails(details.left, *left_line);
        }
        if (right_line) {
            fillBoundaryDetails(details.right, *right_line);
        }

        lane_state_.updateFromProto(details);
        lane_view_model_->updateFromDomain(lane_state_);
        emit laneStateUpdated();
        updateWarnings(lines.timestamp_ms);
    }

    void ConnectionManager::roadObjectsReceivedSlot(const laneproto::RoadObjects& objects) {
        LOG_DEBUG << "RoadObjects (V2) received: objects=" << objects.objects.size()
                  << ", timestamp=" << objects.timestamp_ms;

        // Re-emit for external consumers
        emit roadObjectsReceived(objects);

        // Update marking model from V2 data
        marking_model_.updateFromProtoV2(objects);
        marking_list_model_->updateFromDomain(marking_model_);
        emit markingModelUpdated();

        if (lane_state_.isValid()) {
            const std::uint64_t timestamp_ms = objects.timestamp_ms;
            updateWarnings(timestamp_ms);
        }
    }

}
