#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
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
        Unknown       = 0x00,
        Arrow         = 0x01,
        Crosswalk     = 0x02,
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
    };

    struct MarkingObjects {
        TimestampMs timestamp_ms{};
        SequenceNumber seq{};
        std::vector<MarkingObject> objects;
    };

    class IMessageHandler {
    public: 
        virtual ~IMessageHandler() = default;

        virtual void onLaneSummary(const LaneSummary& msg) = 0;
        virtual void onMarkingObjects(const MarkingObjects& msg) = 0;
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
        void handleLaneSummary();
    };


} // namespace laneproto