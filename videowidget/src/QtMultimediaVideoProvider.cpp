#include "QtMultimediaVideoProvider.hpp"
#include "LoggerMacros.hpp"

#include <QUrl>
#include <QDateTime>

#include <mutex>

using namespace video;

namespace {
    void ensureGStreamerInitialized()
    {
        static std::once_flag flag;
        std::call_once(flag, []() {
            gst_init(nullptr, nullptr);
        });
    }
}

QtMultimediaVideoProvider::QtMultimediaVideoProvider(QObject* parent)
    : IVideoFrameProvider(parent)
{
    ensureGStreamerInitialized();

    m_busTimer.setInterval(250);
    connect(&m_busTimer, &QTimer::timeout, this, &QtMultimediaVideoProvider::pollBusMessages);

    LOG_TRACE << "QtMultimediaVideoProvider created (GStreamer backend)";
}

QtMultimediaVideoProvider::~QtMultimediaVideoProvider()
{
    LOG_TRACE << "QtMultimediaVideoProvider destroyed";
    stop();
}

void QtMultimediaVideoProvider::setSource(const QString& source)
{
    if (m_source == source)
        return;

    m_source = source;
    LOG_INFO << "Source set to " << m_source.toStdString();
    emit sourceChanged(m_source);
}

QString QtMultimediaVideoProvider::source() const
{
    return m_source;
}

void QtMultimediaVideoProvider::start()
{
    if (m_source.isEmpty()) {
        LOG_ERROR << "Source is not set";
        emit errorOccurred("Empty source");
        updateState(ProviderState::Error);
        return;
    }

    if (m_running) {
        LOG_WARN << "Video provider already running";
        return;
    }

    QString error;
    if (!setupPipeline(error)) {
        if (error.isEmpty())
            error = QStringLiteral("Failed to setup GStreamer pipeline");
        emit errorOccurred(error);
        updateState(ProviderState::Error);
        return;
    }

    m_running = true;
    m_fpsTimer.restart();
    m_framesInSecond = 0;
    updateState(ProviderState::Starting);

    GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        QString msg = QStringLiteral("Failed to start GStreamer pipeline");
        emit errorOccurred(msg);
        handlePipelineError(msg);
        return;
    }

    GstState current = GST_STATE_NULL;
    ret = gst_element_get_state(m_pipeline, &current, nullptr, 2 * GST_SECOND);
    if (ret == GST_STATE_CHANGE_FAILURE || current == GST_STATE_NULL) {
        QString msg = QStringLiteral("Pipeline did not transition to PLAYING");
        emit errorOccurred(msg);
        handlePipelineError(msg);
    } else {
        LOG_INFO << "GStreamer pipeline started, waiting for frames...";
    }
}

void QtMultimediaVideoProvider::stop()
{
    if (!m_running && !m_pipeline) {
        return;
    }

    LOG_INFO << "Stopping video provider";
    m_running = false;

    cleanupPipeline();
    resetFps();
    updateState(ProviderState::Stopped);
}

void QtMultimediaVideoProvider::pause()
{
    if (!m_running || !m_pipeline) {
        LOG_WARN << "Cannot pause: not running";
        return;
    }

    LOG_INFO << "Pausing playback";
    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
    updateState(ProviderState::Paused);
}

void QtMultimediaVideoProvider::resume()
{
    if (!m_running || !m_pipeline) {
        LOG_WARN << "Cannot resume: not running";
        return;
    }

    LOG_INFO << "Resuming playback";
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    updateState(ProviderState::Running);
}

bool QtMultimediaVideoProvider::isRunning() const
{
    return m_running;
}

IVideoFrameProvider::ProviderState QtMultimediaVideoProvider::state() const
{
    return m_state;
}

double QtMultimediaVideoProvider::frameRate() const
{
    return m_currentFps;
}

void QtMultimediaVideoProvider::updateState(ProviderState newState)
{
    if (m_state == newState)
        return;

    m_state = newState;
    LOG_INFO << "Provider state changed to" << static_cast<int>(m_state);
    emit stateChanged(m_state);
}

QString QtMultimediaVideoProvider::buildPipelineDescription(QString& error) const
{
    if (m_source.startsWith("gst-pipeline:")) {
        auto desc = m_source.mid(QStringLiteral("gst-pipeline:").size()).trimmed();
        if (desc.isEmpty()) {
            error = QStringLiteral("gst-pipeline: prefix provided but description is empty");
            return {};
        }
        return desc;
    }

    const QUrl url = QUrl::fromUserInput(m_source);
    if (!url.isValid()) {
        error = QStringLiteral("Invalid source URL");
        return {};
    }

    if (url.scheme() == "udp" || url.scheme() == "rtp") {
        const QString host = url.host().isEmpty() ? QStringLiteral("239.0.0.1") : url.host();
        const int port = url.port(5000);
        const QString caps = QStringLiteral("application/x-rtp, media=video, encoding-name=H264, payload=96");

        return QStringLiteral(
                   "udpsrc address=%1 port=%2 auto-multicast=true caps=\"%3\" ! "
                   "rtpjitterbuffer drop-on-latency=true latency=50 ! "
                   "rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! "
                   "video/x-raw,format=RGBA ! "
                   "appsink name=appsink emit-signals=true sync=false max-buffers=1 drop=true")
            .arg(host)
            .arg(port)
            .arg(caps);
    }

    if (url.isLocalFile() || url.scheme().isEmpty()) {
        const QString path = url.isLocalFile() ? url.toLocalFile() : m_source;
        if (path.isEmpty()) {
            error = QStringLiteral("Video source path is empty");
            return {};
        }

        return QStringLiteral(
                   "filesrc location=\"%1\" ! decodebin ! videoconvert ! "
                   "video/x-raw,format=RGBA ! "
                   "appsink name=appsink emit-signals=true sync=false max-buffers=1 drop=true")
            .arg(path);
    }

    return QStringLiteral(
               "uridecodebin uri=\"%1\" ! videoconvert ! video/x-raw,format=RGBA ! "
               "appsink name=appsink emit-signals=true sync=false max-buffers=1 drop=true")
        .arg(m_source);
}

bool QtMultimediaVideoProvider::setupPipeline(QString& error)
{
    cleanupPipeline();

    const QString pipelineDesc = buildPipelineDescription(error);
    if (pipelineDesc.isEmpty()) {
        return false;
    }

    LOG_INFO << "Creating GStreamer pipeline: " << pipelineDesc.toStdString();

    GError* gstError = nullptr;
    m_pipeline = gst_parse_launch(pipelineDesc.toUtf8().constData(), &gstError);
    if (!m_pipeline) {
        error = QStringLiteral("Failed to create pipeline");
        if (gstError) {
            error = QString::fromUtf8(gstError->message);
            g_error_free(gstError);
        }
        return false;
    }

    if (gstError) {
        error = QString::fromUtf8(gstError->message);
        g_error_free(gstError);
    }

    GstElement* sinkElement = gst_bin_get_by_name(GST_BIN(m_pipeline), "appsink");
    if (!sinkElement) {
        error = QStringLiteral("Pipeline does not contain an appsink named 'appsink'");
        cleanupPipeline();
        return false;
    }

    m_appSink = GST_APP_SINK(sinkElement);
    gst_app_sink_set_emit_signals(m_appSink, true);
    gst_app_sink_set_drop(m_appSink, true);
    gst_app_sink_set_max_buffers(m_appSink, 1);

    GstAppSinkCallbacks callbacks{};
    callbacks.new_sample = &QtMultimediaVideoProvider::onNewSample;
    gst_app_sink_set_callbacks(m_appSink, &callbacks, this, nullptr);

    m_bus = gst_element_get_bus(m_pipeline);
    if (m_bus) {
        m_busTimer.start();
    }

    return true;
}

void QtMultimediaVideoProvider::cleanupPipeline()
{
    m_busTimer.stop();

    if (m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_element_get_state(m_pipeline, nullptr, nullptr, GST_SECOND);
    }

    if (m_bus) {
        gst_object_unref(m_bus);
        m_bus = nullptr;
    }

    if (m_appSink) {
        gst_app_sink_set_callbacks(m_appSink, nullptr, nullptr, nullptr);
        gst_object_unref(GST_OBJECT(m_appSink));
        m_appSink = nullptr;
    }

    if (m_pipeline) {
        gst_object_unref(GST_OBJECT(m_pipeline));
        m_pipeline = nullptr;
    }
}

void QtMultimediaVideoProvider::handlePipelineError(const QString& message)
{
    LOG_ERROR << "Pipeline error: " << message.toStdString();
    m_running = false;
    cleanupPipeline();
    resetFps();
    updateState(ProviderState::Error);
}

void QtMultimediaVideoProvider::resetFps()
{
    m_currentFps = 0.0;
    m_framesInSecond = 0;
    m_fpsTimer.invalidate();
}

void QtMultimediaVideoProvider::pollBusMessages()
{
    if (!m_bus)
        return;

    GstMessage* message = nullptr;
    while ((message = gst_bus_pop(m_bus)) != nullptr) {
        handleBusMessage(message);
        gst_message_unref(message);
    }
}

void QtMultimediaVideoProvider::handleBusMessage(GstMessage* message)
{
    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError* err = nullptr;
            gchar* debug = nullptr;
            gst_message_parse_error(message, &err, &debug);

            QString msg = err ? QString::fromUtf8(err->message)
                              : QStringLiteral("Unknown GStreamer error");
            LOG_ERROR << "GStreamer error: " << msg.toStdString();
            if (debug) {
                LOG_ERROR << "Debug info: " << debug;
            }

            if (err)
                g_error_free(err);
            if (debug)
                g_free(debug);

            emit errorOccurred(msg);
            handlePipelineError(msg);
            break;
        }
        case GST_MESSAGE_EOS:
            LOG_WARN << "End of stream received";
            m_running = false;
            cleanupPipeline();
            updateState(ProviderState::Stopped);
            break;
        case GST_MESSAGE_STATE_CHANGED:
            if (GST_MESSAGE_SRC(message) == GST_OBJECT(m_pipeline)) {
                GstState oldState, newState, pending;
                gst_message_parse_state_changed(message, &oldState, &newState, &pending);
                LOG_INFO << "Pipeline state changed from "
                         << gst_element_state_get_name(oldState)
                         << " to "
                         << gst_element_state_get_name(newState);
            }
            break;
        default:
            break;
    }
}

GstFlowReturn QtMultimediaVideoProvider::onNewSample(GstAppSink* sink, gpointer user_data)
{
    auto* self = static_cast<QtMultimediaVideoProvider*>(user_data);
    if (!self || !self->m_running)
        return GST_FLOW_EOS;

    GstSample* sample = gst_app_sink_pull_sample(sink);
    if (!sample)
        return GST_FLOW_EOS;

    self->handleSample(sample);
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

void QtMultimediaVideoProvider::handleSample(GstSample* sample)
{
    GstCaps* caps = gst_sample_get_caps(sample);
    if (!caps) {
        LOG_WARN << "Sample without caps";
        return;
    }

    GstVideoInfo info;
    if (!gst_video_info_from_caps(&info, caps)) {
        LOG_WARN << "Failed to parse video info from caps";
        return;
    }

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    if (!buffer) {
        LOG_WARN << "Sample without buffer";
        return;
    }

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        LOG_WARN << "Failed to map buffer";
        return;
    }

    QImage image(map.data,
                 GST_VIDEO_INFO_WIDTH(&info),
                 GST_VIDEO_INFO_HEIGHT(&info),
                 GST_VIDEO_INFO_PLANE_STRIDE(&info, 0),
                 QImage::Format_RGBA8888);

    QImage copy = image.copy();
    gst_buffer_unmap(buffer, &map);

    FrameHandlePtr handle(new BasicFrameHandle(copy));

    const GstClockTime pts = GST_BUFFER_PTS(buffer);
    const auto ts = GST_CLOCK_TIME_IS_VALID(pts)
                        ? static_cast<int64_t>(GST_TIME_AS_MSECONDS(pts))
                        : QDateTime::currentMSecsSinceEpoch();
    handle->setTimestamp(ts);

    m_framesInSecond++;
    updateFps();

    emit frameReady(handle);
    if (m_state == ProviderState::Starting)
        updateState(ProviderState::Running);
}

void QtMultimediaVideoProvider::updateFps()
{
    const qint64 elapsed = m_fpsTimer.elapsed();

    if (elapsed >= 1000) {
        m_currentFps = (m_framesInSecond * 1000.0) / elapsed;
        LOG_DEBUG << "Current FPS:" << m_currentFps;
        m_framesInSecond = 0;
        m_fpsTimer.restart();
    }
}
