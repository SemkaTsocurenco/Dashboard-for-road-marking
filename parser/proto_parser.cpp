#include "proto_parser.h"
#include "logger/Logger.hpp"
#include "LoggerMacros.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>


namespace {

    inline std::uint16_t read_le_u16(const std::uint8_t* p){
        return static_cast<std::uint16_t>(p[0]) 
            | (static_cast<std::uint16_t>(p[1]) << 8);
    }

    inline std::int16_t read_le_i16(const std::uint8_t* p){
        return static_cast<std::int16_t>(read_le_u16(p));
    }

    inline std::uint32_t read_le_u32(const std::uint8_t* p){
        return static_cast<std::uint32_t>(p[0])
            | (static_cast<std::uint32_t>(p[1]) << 8)
            | (static_cast<std::uint32_t>(p[2]) << 16)
            | (static_cast<std::uint32_t>(p[3]) << 24);
    }

    inline float read_le_float32(const std::uint8_t* p){
        std::uint32_t u32 = read_le_u32(p);
        float f;
        std::memcpy(&f, &u32, sizeof(float));
        return f;
    }

    inline std::uint16_t crc16_ibm (const std::uint8_t* data, std::size_t len){
        std::uint16_t crc = 0xFFFF;

        for (std::size_t i = 0; i < len; ++i){
            crc ^= data[i];
            for (int bit = 0; bit < 8; ++bit){
                if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
                else crc >>=1;
            }
        }
        return crc;
    }
}

namespace laneproto {

    ProtoParser::ProtoParser(IMessageHandler& handler) noexcept
        : handler_(handler) {
    }

    void ProtoParser::reset() noexcept {
        state_ = State::WaitingSync;
        header_pos_ = 0;
        crc_pos_ = 0;
        payload_pos_ = 0; 
        payload_buf_.clear(); 
        current_header_ = FrameHeader{};
    }

    bool ProtoParser::parseHeaderFromBuffer() {
        FrameHeader h;

        h.ver = header_buf_[0];
        if (h.ver != kProtocolVersion) {
            ParseError err;
            err.code = ParseErrorCode::BadVersion;
            err.message = "Unsupported protocol version: " + std::to_string(h.ver);
            handler_.onParseError(err);
            return false;
        }

        h.msg_type = static_cast<MsgType>(header_buf_[1]);
        if (h.msg_type != MsgType::MarkingObjects &&
            h.msg_type != MsgType::LaneSummary &&
            h.msg_type != MsgType::LaneDetails &&
            h.msg_type != MsgType::MarkingObjectsEx &&
            h.msg_type != MsgType::FittedLines){
            ParseError err;
            err.code = ParseErrorCode::UnknownMsgType;
            err.message = "Unknown MSG_TYPE: " + std::to_string(
                static_cast<std::uint8_t>(h.msg_type));
            handler_.onParseError(err);
            return false;
        }

        h.seq = header_buf_[2];
        h.timestamp_ms = read_le_u32(header_buf_ + 3);
        h.payload_len = read_le_u16(header_buf_ + 7);
        if (h.payload_len > kMaxPayloadLength){
            ParseError err;
            err.code = ParseErrorCode::PayloadTooLong;
            err.message = "Payload too long: " + std::to_string(h.payload_len);
            handler_.onParseError(err);
            return false; 
        }

        current_header_ = h;

        return true;
    }

    bool ProtoParser::verifyCrc(){
        std::uint16_t received_crc = read_le_u16(crc_buf_);

        std::vector<std::uint8_t> tmp;
        tmp.reserve(kHeaderSize + current_header_.payload_len);

        tmp.insert(tmp.end(), header_buf_, header_buf_ + kHeaderSize);
        tmp.insert(tmp.end(), payload_buf_.begin(), payload_buf_.end());

        std::uint16_t calc_crc = crc16_ibm(tmp.data(), tmp.size());

        if (calc_crc != received_crc){
            ParseError err;
            err.code = ParseErrorCode::CrcMismatch;
            err.message = "CRC mismatch";
            handler_.onParseError(err);
            return false;
        }

        return true;
    }

    void ProtoParser::handleMarkingObjects () {
        if (payload_buf_.size() != current_header_.payload_len) {
            ParseError err;
            err.code = ParseErrorCode::MarkingFormat;
            err.message = "Payload size mismatch for MarkingObjects";
            handler_.onParseError(err);
            return;
        }
        if (payload_buf_.empty()) {
            ParseError err;
            err.code = ParseErrorCode::MarkingFormat;
            err.message = "Empty payload for MarkingObjects";
            handler_.onParseError(err);
            return;
        }

        std::uint8_t num_objects = payload_buf_[0];

        std::size_t expected_len = 1u + static_cast<std::size_t>(num_objects) * 13u;
        if (current_header_.payload_len != expected_len){
            ParseError err;
            err.code = ParseErrorCode::MarkingFormat;
            err.message = "MarkingObjects LEN mismatch: expected "
                + std::to_string(expected_len) + ", got "
                + std::to_string(current_header_.payload_len);
            handler_.onParseError(err);
            return;
        }

        MarkingObjects msg;
        msg.timestamp_ms = current_header_.timestamp_ms;
        msg.seq = current_header_.seq;
        msg.objects.clear();
        msg.objects.reserve(num_objects);

        const std::size_t object_size = 13;
        std::size_t offset = 1;

        for (std::uint8_t i = 0; i < num_objects; ++i){
            if (offset + object_size > payload_buf_.size()){
                ParseError err;
                err.code = ParseErrorCode::MarkingFormat;
                err.message = "Truncated MarkingObject in payload";
                handler_.onParseError(err);
                return;
            }

            const std::uint8_t* p = payload_buf_.data() + offset;
            MarkingObject obj;
            obj.class_id = static_cast<MarkingClassId>(p[0]);

            std::int16_t x_decim = static_cast<std::int16_t>(read_le_u16(p + 1));
            std::int16_t y_decim = static_cast<std::int16_t>(read_le_u16(p + 3));
            std::uint16_t length_decim = read_le_u16(p + 5);
            std::uint16_t width_decim = read_le_u16(p + 7);
            std::int16_t yaw_decideg = static_cast<std::int16_t>(read_le_u16(p + 9));

            obj.x_m = static_cast<float>(x_decim) / 10.0f;
            obj.y_m = static_cast<float>(y_decim) / 10.0f;
            obj.length_m = static_cast<float>(length_decim) / 10.0f;
            obj.width_m = static_cast<float>(width_decim) / 10.0f;
            obj.yaw_deg = static_cast<float>(yaw_decideg) / 10.0f;

            obj.confidence = p[11];
            obj.flags = p[12];
            obj.line_color = LineColor::Unknown;
            obj.line_style = LineStyle::Unknown;

            msg.objects.push_back(obj);

            offset += object_size;
        }
        handler_.onMarkingObjects(msg);
    }

    void ProtoParser::handleMarkingObjectsEx() {
        if (payload_buf_.size() != current_header_.payload_len) {
            ParseError err;
            err.code = ParseErrorCode::MarkingExFormat;
            err.message = "Payload size mismatch for MarkingObjectsEx";
            handler_.onParseError(err);
            return;
        }
        if (payload_buf_.empty()) {
            ParseError err;
            err.code = ParseErrorCode::MarkingExFormat;
            err.message = "Empty payload for MarkingObjectsEx";
            handler_.onParseError(err);
            return;
        }

        std::uint8_t num_objects = payload_buf_[0];

        const std::size_t object_size = 15;
        std::size_t expected_len = 1u + static_cast<std::size_t>(num_objects) * object_size;
        if (current_header_.payload_len != expected_len){
            ParseError err;
            err.code = ParseErrorCode::MarkingExFormat;
            err.message = "MarkingObjectsEx LEN mismatch: expected "
                + std::to_string(expected_len) + ", got "
                + std::to_string(current_header_.payload_len);
            handler_.onParseError(err);
            return;
        }

        MarkingObjects msg;
        msg.timestamp_ms = current_header_.timestamp_ms;
        msg.seq = current_header_.seq;
        msg.objects.clear();
        msg.objects.reserve(num_objects);

        std::size_t offset = 1;

        for (std::uint8_t i = 0; i < num_objects; ++i){
            if (offset + object_size > payload_buf_.size()){
                ParseError err;
                err.code = ParseErrorCode::MarkingExFormat;
                err.message = "Truncated MarkingObjectEx in payload";
                handler_.onParseError(err);
                return;
            }

            const std::uint8_t* p = payload_buf_.data() + offset;
            MarkingObject obj;
            obj.class_id = static_cast<MarkingClassId>(p[0]);

            std::int16_t x_decim = static_cast<std::int16_t>(read_le_u16(p + 1));
            std::int16_t y_decim = static_cast<std::int16_t>(read_le_u16(p + 3));
            std::uint16_t length_decim = read_le_u16(p + 5);
            std::uint16_t width_decim = read_le_u16(p + 7);
            std::int16_t yaw_decideg = static_cast<std::int16_t>(read_le_u16(p + 9));

            obj.x_m = static_cast<float>(x_decim) / 10.0f;
            obj.y_m = static_cast<float>(y_decim) / 10.0f;
            obj.length_m = static_cast<float>(length_decim) / 10.0f;
            obj.width_m = static_cast<float>(width_decim) / 10.0f;
            obj.yaw_deg = static_cast<float>(yaw_decideg) / 10.0f;

            obj.line_color = static_cast<LineColor>(p[11]);
            obj.line_style = static_cast<LineStyle>(p[12]);

            obj.confidence = p[13];
            obj.flags = p[14];

            msg.objects.push_back(obj);

            offset += object_size;
        }
        handler_.onMarkingObjectsEx(msg);
    }

    void ProtoParser::handleLaneSummary (){
        if (payload_buf_.size() != current_header_.payload_len) {
            ParseError err;
            err.code = ParseErrorCode::LaneSummaryFormat;
            err.message = "Payload size mismatch for LaneSummary";
            handler_.onParseError(err);
            return;
        }

        if (current_header_.payload_len != 8u) {
            ParseError err;
            err.code = ParseErrorCode::LaneSummaryFormat;
            err.message = "LaneSummary LEN must be 8, got "
                + std::to_string(current_header_.payload_len);
            handler_.onParseError(err);
            return;
        }

        const std::uint8_t* p = payload_buf_.data();

        std::int16_t left_decim = static_cast<std::int16_t>(read_le_u16(p + 0));
        std::int16_t right_decim = static_cast<std::int16_t>(read_le_u16(p + 2));
        float left_m = static_cast<float>(left_decim) / 10.0f;
        float right_m = static_cast<float>(right_decim) / 10.0f;

        LaneType lane_left = static_cast<LaneType>(p[4]);
        LaneType lane_right = static_cast<LaneType>(p[5]);

        std::uint8_t allowed_maneuvers = p[6];      
        std::uint8_t quality = p[7];

        LaneSummary msg;
        msg.timestamp_ms    = current_header_.timestamp_ms;
        msg.seq             = current_header_.seq;
        msg.left_offset_m   = left_m;
        msg.right_offset_m  = right_m;
        msg.lane_type_left  = lane_left;
        msg.lane_type_right = lane_right;
        msg.allowed_maneuvers = allowed_maneuvers;
        msg.quality           = quality;

        handler_.onLaneSummary(msg);
    }

    void ProtoParser::handleLaneDetails (){
        constexpr std::size_t kFixedPart = 18; // offsets, flags, types, colors, widths, qualities, point counts
        constexpr std::size_t kPointsPerSide = 3;
        constexpr std::size_t kPointSizeBytes = 4; // int16 x + int16 y
        constexpr std::size_t kExpectedMin = kFixedPart + 2 * kPointsPerSide * kPointSizeBytes; // 42 bytes

        if (payload_buf_.size() < kExpectedMin) {
            ParseError err;
            err.code = ParseErrorCode::LaneDetailsFormat;
            err.message = "Payload too small for LaneDetails: got "
                + std::to_string(payload_buf_.size()) + ", expected at least "
                + std::to_string(kExpectedMin);
            handler_.onParseError(err);
            return;
        }

        auto remaining = payload_buf_.size();
        if (remaining != current_header_.payload_len) {
            ParseError err;
            err.code = ParseErrorCode::LaneDetailsFormat;
            err.message = "Payload size mismatch for LaneDetails";
            handler_.onParseError(err);
            return;
        }

        LaneDetails msg;
        msg.timestamp_ms = current_header_.timestamp_ms;
        msg.seq = current_header_.seq;

        std::size_t offset = 0;

        auto read_i16_and_advance = [&](float& out){
            if (offset + 2 > payload_buf_.size())
                return false;
            out = static_cast<float>(read_le_i16(payload_buf_.data() + offset)) / 10.0f;
            offset += 2;
            return true;
        };

        if (!read_i16_and_advance(msg.left_offset_m) ||
            !read_i16_and_advance(msg.right_offset_m)) {
            ParseError err;
            err.code = ParseErrorCode::LaneDetailsFormat;
            err.message = "Truncated offsets in LaneDetails";
            handler_.onParseError(err);
            return;
        }

        if (offset + 10 > payload_buf_.size()) { // allowed, quality, lane types, colors, widths, qualities
            ParseError err;
            err.code = ParseErrorCode::LaneDetailsFormat;
            err.message = "Truncated main fields in LaneDetails";
            handler_.onParseError(err);
            return;
        }

        msg.allowed_maneuvers = payload_buf_[offset++];
        msg.quality = payload_buf_[offset++];

        msg.left.type = static_cast<LaneType>(payload_buf_[offset++]);
        msg.right.type = static_cast<LaneType>(payload_buf_[offset++]);

        msg.left.color = static_cast<LineColor>(payload_buf_[offset++]);
        msg.right.color = static_cast<LineColor>(payload_buf_[offset++]);

        if (offset + 4 > payload_buf_.size()) {
            ParseError err;
            err.code = ParseErrorCode::LaneDetailsFormat;
            err.message = "Truncated width fields in LaneDetails";
            handler_.onParseError(err);
            return;
        }

        std::uint16_t left_width_decim = read_le_u16(payload_buf_.data() + offset);
        offset += 2;
        std::uint16_t right_width_decim = read_le_u16(payload_buf_.data() + offset);
        offset += 2;

        msg.left.width_m = static_cast<float>(left_width_decim) / 10.0f;
        msg.right.width_m = static_cast<float>(right_width_decim) / 10.0f;

        if (offset + 4 > payload_buf_.size()) {
            ParseError err;
            err.code = ParseErrorCode::LaneDetailsFormat;
            err.message = "Truncated quality/point-count fields in LaneDetails";
            handler_.onParseError(err);
            return;
        }

        msg.left.quality = payload_buf_[offset++];
        msg.right.quality = payload_buf_[offset++];

        msg.left.points_count = std::min<std::uint8_t>(payload_buf_[offset++], kPointsPerSide);
        msg.right.points_count = std::min<std::uint8_t>(payload_buf_[offset++], kPointsPerSide);

        auto read_points = [&](LaneBoundaryDetails& boundary, const char* side)->bool {
            for (std::size_t i = 0; i < kPointsPerSide; ++i) {
                if (offset + kPointSizeBytes > payload_buf_.size()) {
                    ParseError err;
                    err.code = ParseErrorCode::LaneDetailsFormat;
                    err.message = std::string("Truncated point data for ") + side;
                    handler_.onParseError(err);
                    return false;
                }
                std::int16_t x_decim = read_le_i16(payload_buf_.data() + offset);
                std::int16_t y_decim = read_le_i16(payload_buf_.data() + offset + 2);
                boundary.points[i].x_m = static_cast<float>(x_decim) / 10.0f;
                boundary.points[i].y_m = static_cast<float>(y_decim) / 10.0f;
                offset += kPointSizeBytes;
            }
            return true;
        };

        if (!read_points(msg.left, "left boundary") ||
            !read_points(msg.right, "right boundary")) {
            return;
        }

        if (payload_buf_.size() > offset) {
            LOG_WARN << "LaneDetails payload has " << (payload_buf_.size() - offset)
                     << " extra bytes, ignoring trailing data";
        }

        handler_.onLaneDetails(msg);
    }

    void ProtoParser::handleFittedLines() {
        if (payload_buf_.size() != current_header_.payload_len) {
            ParseError err;
            err.code = ParseErrorCode::FittedLinesFormat;
            err.message = "Payload size mismatch for FittedLines";
            handler_.onParseError(err);
            return;
        }
        if (payload_buf_.empty()) {
            ParseError err;
            err.code = ParseErrorCode::FittedLinesFormat;
            err.message = "Empty payload for FittedLines";
            handler_.onParseError(err);
            return;
        }

        std::uint8_t num_lines = payload_buf_[0];

        const std::size_t line_size = 23;
        std::size_t expected_len = 1u + static_cast<std::size_t>(num_lines) * line_size;
        if (current_header_.payload_len != expected_len){
            ParseError err;
            err.code = ParseErrorCode::FittedLinesFormat;
            err.message = "FittedLines LEN mismatch: expected "
                + std::to_string(expected_len) + ", got "
                + std::to_string(current_header_.payload_len);
            handler_.onParseError(err);
            return;
        }

        FittedLines msg;
        msg.timestamp_ms = current_header_.timestamp_ms;
        msg.seq = current_header_.seq;
        msg.lines.clear();
        msg.lines.reserve(num_lines);

        std::size_t offset = 1;

        for (std::uint8_t i = 0; i < num_lines; ++i){
            if (offset + line_size > payload_buf_.size()){
                ParseError err;
                err.code = ParseErrorCode::FittedLinesFormat;
                err.message = "Truncated FittedLine in payload";
                handler_.onParseError(err);
                return;
            }

            const std::uint8_t* p = payload_buf_.data() + offset;
            FittedLine line;

            line.class_id = static_cast<MarkingClassId>(p[0]);
            line.side = static_cast<LineSide>(p[1]);
            line.color = static_cast<LineColor>(p[2]);
            line.style = static_cast<LineStyle>(p[3]);

            line.poly_a = read_le_float32(p + 4);
            line.poly_b = read_le_float32(p + 8);
            line.poly_c = read_le_float32(p + 12);

            line.y_start = read_le_i16(p + 16);
            line.y_end = read_le_i16(p + 18);

            line.confidence = p[20];
            line.quality = p[21];

            msg.lines.push_back(line);

            offset += line_size;
        }
        handler_.onFittedLines(msg);
    }

    void ProtoParser::feed(const std::vector<std::uint8_t>& data) {
        feed(data.data(), data.size());
    }

    void ProtoParser::feed(const std::uint8_t* data, std::size_t size) {
        for (std::size_t i = 0; i < size; ++i) {
            std::uint8_t byte = data[i];

            switch (state_) {
                case State::WaitingSync:
                    if (byte == kSyncByte){
                        header_pos_ = 0;
                        state_ = State::ReadingHeader;
                    }
                    break;
                case State::ReadingHeader:
                    header_buf_[header_pos_++] = byte;
                    if (header_pos_ == kHeaderSize){
                        header_pos_ = 0;
                        if (!parseHeaderFromBuffer()){
                            reset();
                            break;
                        }
                        payload_buf_.assign(current_header_.payload_len, 0);
                        payload_pos_ = 0;

                        state_ = State::ReadingPayload;
                    }
                    break;
                case State::ReadingPayload:
                    payload_buf_[payload_pos_++] = byte;
                    if (payload_pos_ == current_header_.payload_len){
                        crc_pos_ = 0;
                        state_ = State::ReadingCrc;
                    }
                    break;
                case State::ReadingCrc:
                    crc_buf_[crc_pos_++] = byte;
                    if (crc_pos_ == 2) {
                        crc_pos_ = 0;
                        if (!verifyCrc()){
                            reset();
                            break;
                        }

                        switch (current_header_.msg_type) {
                            case MsgType::LaneSummary:
                                handleLaneSummary();
                                break;
                            case MsgType::MarkingObjects:
                                handleMarkingObjects();
                                break;
                            case MsgType::LaneDetails:
                                handleLaneDetails();
                                break;
                            case MsgType::MarkingObjectsEx:
                                handleMarkingObjectsEx();
                                break;
                            case MsgType::FittedLines:
                                handleFittedLines();
                                break;
                        }
                        reset();
                    }
                    break;
            }
        }
    }
} // namespace laneproto
