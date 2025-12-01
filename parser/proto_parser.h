#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <array>
#include "../logger/Logger.hpp"

namespace laneproto {
    using TimestampMs = std::uint32_t;
    using SequenceNumber = std::uint8_t;

    constexpr std::uint8_t kProtocolVersion = 0x01;
    constexpr std::uint8_t kSyncByte = 0xAA;
    constexpr std::size_t kMaxPayloadLength = 1024;

    enum class MsgType : std::uint8_t {
        LaneSummary     = 0x01,
        MarkingObjects  = 0x02,
        LaneDetails     = 0x03,
        MarkingObjectsEx= 0x04,
        FittedLines     = 0x05,
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

    class IMessageHandler {
    public:
        virtual ~IMessageHandler() = default;

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
            MsgType       msg_type    = MsgType::LaneSummary;
            SequenceNumber seq        = 0;
            TimestampMs   timestamp_ms{};
            std::uint16_t payload_len = 0;
        };

        IMessageHandler& handler_;

        State state_{State::WaitingSync};

        static constexpr std::size_t kHeaderSize = 1 + 1 + 1 + 4 + 2;

        std::uint8_t header_buf_[kHeaderSize]{};
        std::size_t  header_pos_ = 0;

        FrameHeader current_header_{};

        std::vector<std::uint8_t> payload_buf_;
        std::size_t               payload_pos_ = 0;

        std::uint8_t crc_buf_[2]{};
        std::size_t  crc_pos_ = 0;

        bool parseHeaderFromBuffer();
        bool verifyCrc();
        void handleMarkingObjects();
        void handleMarkingObjectsEx();
        void handleLaneSummary();
        void handleLaneDetails();
        void handleFittedLines();
    };


} // namespace laneproto
