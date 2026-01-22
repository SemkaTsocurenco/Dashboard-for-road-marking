#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <array>
#include "../logger/Logger.hpp"
#include "AppConfig.hpp"

namespace laneproto {
    using TimestampMs = std::uint32_t;
    using SequenceNumber = std::uint8_t;

    constexpr std::uint8_t kProtocolVersion = config::ProtocolConfig::PROTOCOL_VERSION;
    constexpr std::uint8_t kSyncByte = config::ProtocolConfig::SYNC_BYTE;
    constexpr std::size_t kMaxPayloadLength = config::ProtocolConfig::MAX_PAYLOAD_LENGTH;

    // V2 Protocol Message Types
    enum class MsgType : std::uint8_t {
        LaneLines       = 0x01,  // V2: Lane lines with polynomials and points in meters
        RoadObjects     = 0x02,  // V2: Road marking objects (arrows, crosswalks, etc.)
        // Legacy V1 types (for reference, no longer used)
        // LaneSummary     = 0x01,
        // MarkingObjects  = 0x02,
        // LaneDetails     = 0x03,
        // MarkingObjectsEx= 0x04,
        // FittedLines     = 0x05,
    };

    enum class LaneType : std::uint8_t {
        Unknown       = 0x00,
        Solid         = 0x01,
        Dashed        = 0x02,
        DoubleSolid   = 0x03,
        DoubleDashed  = 0x04,
        SolidDashed   = 0x05,
    };

    enum class MarkingClassId : std::uint8_t {
        Unknown              = 0x00,
        BoxJunction          = 0x01,
        Crosswalk            = 0x02,
        StopLine             = 0x03,
        SolidSingleWhite     = 0x04,
        SolidSingleYellow    = 0x05,
        SolidSingleRed       = 0x06,
        DoubleWhite          = 0x07,
        DoubleYellow         = 0x08,
        DashedWhite          = 0x09,
        DashedYellow         = 0x0A,
        ArrowLeft            = 0x0B,
        ArrowStraight        = 0x0C,
        ArrowRight           = 0x0D,
        ArrowLeftStraight    = 0x0E,
        ArrowRightStraight   = 0x0F,
        ChannelizingLine     = 0x10,
        MotorIcon            = 0x16,
        BikeIcon             = 0x17,
    };

    enum class LineColor : std::uint8_t {
        Unknown = 0x00,
        White   = 0x01,
        Yellow  = 0x02,
        Red     = 0x03,
    };

    enum class LineStyle : std::uint8_t {
        Unknown = 0x00,
        Solid   = 0x01,
        Dashed  = 0x02,
        Double  = 0x03,
    };

    enum class LineSide : std::uint8_t {
        Unknown = 0x00,
        Left    = 0x01,
        Right   = 0x02,
        Center  = 0x03,
    };

    enum class ParseErrorCode {
        Unknown,
        BadVersion,
        PayloadTooLong,
        HeaderTruncated,
        PayloadTruncated,
        CrcMismatch,
        UnknownMsgType,
        LaneSummaryFormat,
        MarkingFormat,
        LaneDetailsFormat,
        MarkingExFormat,
        FittedLinesFormat,
    };

    struct ParseError {
        ParseErrorCode code{};
        std::string message;
    };

    struct LaneSummary {
        TimestampMs timestamp_ms{};
        SequenceNumber seq{};
        float left_offset_m = 0.0f;
        float right_offset_m = 0.0f;
        LaneType lane_type_left = LaneType::Unknown;
        LaneType lane_type_right = LaneType::Unknown;
        std::uint8_t allowed_maneuvers = 0;
        std::uint8_t quality = 0;
    };

    struct MarkingObject {
        MarkingClassId class_id = MarkingClassId::Unknown;
        float x_m = 0.0f;
        float y_m = 0.0f;
        float length_m = 0.0f;
        float width_m = 0.0f;
        float yaw_deg = 0.0f;
        std::uint8_t confidence = 0;
        std::uint8_t flags = 0;
        LineColor line_color = LineColor::Unknown;
        LineStyle line_style = LineStyle::Unknown;
    };

    struct MarkingObjects {
        TimestampMs timestamp_ms{};
        SequenceNumber seq{};
        std::vector<MarkingObject> objects;
    };

    struct LanePoint {
        float x_m = 0.0f;
        float y_m = 0.0f;
    };

    struct LaneBoundaryDetails {
        LaneType type = LaneType::Unknown;
        LineColor color = LineColor::Unknown;
        float width_m = 0.0f;
        std::uint8_t quality = 0;
        std::uint8_t points_count = 0;
        std::array<LanePoint, 3> points{};
    };

    struct LaneDetails {
        TimestampMs timestamp_ms{};
        SequenceNumber seq{};
        float left_offset_m = 0.0f;
        float right_offset_m = 0.0f;
        std::uint8_t allowed_maneuvers = 0;
        std::uint8_t quality = 0;
        LaneBoundaryDetails left;
        LaneBoundaryDetails right;
    };

    struct FittedLine {
        MarkingClassId class_id = MarkingClassId::Unknown;
        LineSide side = LineSide::Unknown;
        LineColor color = LineColor::Unknown;
        LineStyle style = LineStyle::Unknown;
        float poly_a = 0.0f;
        float poly_b = 0.0f;
        float poly_c = 0.0f;
        std::int16_t y_start = 0;
        std::int16_t y_end = 0;
        std::uint8_t confidence = 0;
        std::uint8_t quality = 0;
    };

    struct FittedLines {
        TimestampMs timestamp_ms{};
        SequenceNumber seq{};
        std::vector<FittedLine> lines;
    };

    // ============================================
    // V2 Protocol Data Structures
    // ============================================

    // V2: Point in meters (real-world coordinates)
    struct PointMeters {
        float x_m = 0.0f;
        float y_m = 0.0f;
    };

    // V2: Point in pixels (image coordinates)
    struct PointPixels {
        float x_px = 0.0f;
        float y_px = 0.0f;
    };

    // V2: Lane line with polynomial coefficients and 3 points
    // Structure size: 71 bytes per line
    struct LaneLine {
        LineSide side = LineSide::Unknown;      // 0=unknown, 1=left, 2=right, 3=center
        LineStyle style = LineStyle::Unknown;   // 0=unknown, 1=solid, 2=dashed, 3=double
        LineColor color = LineColor::Unknown;   // 0=unknown, 1=white, 2=yellow, 3=red

        // Polynomial coefficients: x = a*y² + b*y + c
        float poly_a = 0.0f;
        float poly_b = 0.0f;
        float poly_c = 0.0f;

        // Center of line in meters
        float center_x_m = 0.0f;
        float center_y_m = 0.0f;

        // Three points in meters (top, middle, bottom)
        std::array<PointMeters, 3> points_m{};

        // Three points in pixels (for overlay/debug)
        std::array<PointPixels, 3> points_px{};
    };

    // V2: Lane lines message
    struct LaneLines {
        TimestampMs timestamp_ms{};
        SequenceNumber seq{};
        std::vector<LaneLine> lines;
    };

    // V2: Road object (arrows, crosswalks, stop lines, etc.)
    // Structure size: 41 bytes per object
    struct RoadObject {
        MarkingClassId class_id = MarkingClassId::Unknown;
        float center_x_m = 0.0f;    // Center X in meters
        float center_y_m = 0.0f;    // Center Y in meters (forward from camera)
        float length_m = 0.0f;      // Length in meters
        float width_m = 0.0f;       // Width in meters
        float yaw_rad = 0.0f;       // Orientation in radians (0 = forward)
        float center_x_px = 0.0f;   // Center X in pixels (OpenCV coords)
        float center_y_px = 0.0f;   // Center Y in pixels (OpenCV coords)
        float width_px = 0.0f;      // Width in pixels
        float length_px = 0.0f;     // Length in pixels
        std::uint8_t confidence = 0; // Detection confidence (0-255)
        std::uint8_t flags = 0;      // State flags (bitmask)
        std::uint16_t reserved = 0;  // Reserved for future use
    };

    // V2: Road objects message
    struct RoadObjects {
        TimestampMs timestamp_ms{};
        SequenceNumber seq{};
        std::vector<RoadObject> objects;
    };

    class IMessageHandler {
    public:
        virtual ~IMessageHandler() = default;

        // V2 Protocol handlers
        virtual void onLaneLines(const LaneLines& msg) = 0;
        virtual void onRoadObjects(const RoadObjects& msg) = 0;

        // Legacy V1 handlers (kept for compatibility, implementations can be empty)
        virtual void onLaneSummary(const LaneSummary& msg) = 0;
        virtual void onMarkingObjects(const MarkingObjects& msg) = 0;
        virtual void onLaneDetails(const LaneDetails& msg) = 0;
        virtual void onMarkingObjectsEx(const MarkingObjects& msg) = 0;
        virtual void onFittedLines(const FittedLines& msg) = 0;

        virtual void onParseError(const ParseError& error) = 0;
    };

    class ProtoParser {
    public:
        explicit ProtoParser(IMessageHandler& handler) noexcept;
        ~ProtoParser() = default;

        void feed(const std::vector<std::uint8_t>& data);
        void feed(const std::uint8_t* data, std::size_t size);
        void reset() noexcept;

        ProtoParser(const ProtoParser&) = delete;
        ProtoParser& operator=(const ProtoParser&) = delete;
        ProtoParser(ProtoParser&&) = delete;
        ProtoParser& operator=(ProtoParser&&) = delete;

    private:
        enum class State {
            WaitingSync,
            ReadingHeader,
            ReadingPayload,
            ReadingCrc,
        };

        struct FrameHeader {
            std::uint8_t  ver         = 0x00;
            MsgType       msg_type    = MsgType::LaneLines;  // V2 default
            SequenceNumber seq        = 0;
            TimestampMs   timestamp_ms{};
            std::uint16_t payload_len = 0;
        };

        IMessageHandler& handler_;

        State state_{State::WaitingSync};

        static constexpr std::size_t kHeaderSize = config::ProtocolConfig::HEADER_SIZE;

        std::uint8_t header_buf_[kHeaderSize]{};
        std::size_t  header_pos_ = 0;

        FrameHeader current_header_{};

        std::vector<std::uint8_t> payload_buf_;
        std::size_t               payload_pos_ = 0;

        std::uint8_t crc_buf_[2]{};
        std::size_t  crc_pos_ = 0;

        bool parseHeaderFromBuffer();
        bool verifyCrc();

        // V2 Protocol handlers
        void handleLaneLines();
        void handleRoadObjects();

        // Legacy V1 handlers (not used in V2)
        void handleMarkingObjects();
        void handleMarkingObjectsEx();
        void handleLaneSummary();
        void handleLaneDetails();
        void handleFittedLines();
    };


} // namespace laneproto
