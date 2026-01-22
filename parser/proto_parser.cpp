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

    inline laneproto::LineColor decode_line_color(std::uint8_t raw) {
        // Protocol: 0=white, 1=yellow, 2=red
        switch (raw) {
            case 0: return laneproto::LineColor::White;
            case 1: return laneproto::LineColor::Yellow;
            case 2: return laneproto::LineColor::Red;
            default: return laneproto::LineColor::Unknown;
        }
    }

    inline laneproto::LineStyle decode_line_style(std::uint8_t raw) {
        // Protocol: 0=unknown, 1=solid, 2=double, 3=dashed
        switch (raw) {
            case 1: return laneproto::LineStyle::Solid;
            case 2: return laneproto::LineStyle::Double;
            case 3: return laneproto::LineStyle::Dashed;
            default: return laneproto::LineStyle::Unknown;
        }
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
        // V2 Protocol: Only LaneLines (0x01) and RoadObjects (0x02) are valid
        if (h.msg_type != MsgType::LaneLines &&
            h.msg_type != MsgType::RoadObjects) {
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

            obj.line_color = decode_line_color(p[11]);
            obj.line_style = decode_line_style(p[12]);

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
        // Current NN sender packs LaneDetails as:
        // left_type, right_type, left_color, right_color,
        // left_quality, right_quality, left_width_dm, right_width_dm,
        // then 3 points per side (x_dm, y_dm) each int16.
        constexpr std::size_t kFixedPart = 10; // types, colors, qualities, widths
        constexpr std::size_t kPointsPerSide = 3;
        constexpr std::size_t kPointSizeBytes = 4; // int16 x + int16 y
        constexpr std::size_t kExpectedMin = kFixedPart + 2 * kPointsPerSide * kPointSizeBytes; // 34 bytes

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
        msg.left_offset_m = 0.0f;
        msg.right_offset_m = 0.0f;
        msg.allowed_maneuvers = 0;
        msg.quality = 0;

        std::size_t offset = 0;

        msg.left.type = static_cast<LaneType>(payload_buf_[offset++]);
        msg.right.type = static_cast<LaneType>(payload_buf_[offset++]);

        msg.left.color = decode_line_color(payload_buf_[offset++]);
        msg.right.color = decode_line_color(payload_buf_[offset++]);

        if (offset + 2 > payload_buf_.size()) {
            ParseError err;
            err.code = ParseErrorCode::LaneDetailsFormat;
            err.message = "Truncated quality fields in LaneDetails";
            handler_.onParseError(err);
            return;
        }

        msg.left.quality = payload_buf_[offset++];
        msg.right.quality = payload_buf_[offset++];

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

        msg.left.points_count = kPointsPerSide;
        msg.right.points_count = kPointsPerSide;

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

        const std::size_t line_size = 22; // matches NN sender packing "<BBBBfffhhBB"
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
            line.color = decode_line_color(p[2]);
            line.style = decode_line_style(p[3]);

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

    // ============================================
    // V2 Protocol Handlers
    // ============================================

    void ProtoParser::handleLaneLines() {
        if (payload_buf_.size() != current_header_.payload_len) {
            ParseError err;
            err.code = ParseErrorCode::FittedLinesFormat;
            err.message = "Payload size mismatch for LaneLines";
            handler_.onParseError(err);
            return;
        }
        if (payload_buf_.empty()) {
            ParseError err;
            err.code = ParseErrorCode::FittedLinesFormat;
            err.message = "Empty payload for LaneLines";
            handler_.onParseError(err);
            return;
        }

        std::uint8_t num_lines = payload_buf_[0];

        // V2: Each line is 71 bytes
        const std::size_t line_size = 71;
        std::size_t expected_len = 1u + static_cast<std::size_t>(num_lines) * line_size;
        if (current_header_.payload_len != expected_len) {
            ParseError err;
            err.code = ParseErrorCode::FittedLinesFormat;
            err.message = "LaneLines LEN mismatch: expected "
                + std::to_string(expected_len) + ", got "
                + std::to_string(current_header_.payload_len);
            handler_.onParseError(err);
            return;
        }

        LaneLines msg;
        msg.timestamp_ms = current_header_.timestamp_ms;
        msg.seq = current_header_.seq;
        msg.lines.clear();
        msg.lines.reserve(num_lines);

        std::size_t offset = 1;

        for (std::uint8_t i = 0; i < num_lines; ++i) {
            if (offset + line_size > payload_buf_.size()) {
                ParseError err;
                err.code = ParseErrorCode::FittedLinesFormat;
                err.message = "Truncated LaneLine in payload";
                handler_.onParseError(err);
                return;
            }

            const std::uint8_t* p = payload_buf_.data() + offset;
            LaneLine line;

            // V2 format: side(1), style(1), color(1), poly_a/b/c(12), x_m/y_m(8),
            // 3 points meters(24), 3 points pixels(24) = 71 bytes
            line.side = static_cast<LineSide>(p[0]);
            line.style = decode_line_style(p[1]);
            line.color = decode_line_color(p[2]);

            line.poly_a = read_le_float32(p + 3);
            line.poly_b = read_le_float32(p + 7);
            line.poly_c = read_le_float32(p + 11);

            line.center_x_m = read_le_float32(p + 15);
            line.center_y_m = read_le_float32(p + 19);

            // 3 points in meters (top, middle, bottom)
            line.points_m[0].x_m = read_le_float32(p + 23);
            line.points_m[0].y_m = read_le_float32(p + 27);
            line.points_m[1].x_m = read_le_float32(p + 31);
            line.points_m[1].y_m = read_le_float32(p + 35);
            line.points_m[2].x_m = read_le_float32(p + 39);
            line.points_m[2].y_m = read_le_float32(p + 43);

            // 3 points in pixels (for overlay/debug)
            line.points_px[0].x_px = read_le_float32(p + 47);
            line.points_px[0].y_px = read_le_float32(p + 51);
            line.points_px[1].x_px = read_le_float32(p + 55);
            line.points_px[1].y_px = read_le_float32(p + 59);
            line.points_px[2].x_px = read_le_float32(p + 63);
            line.points_px[2].y_px = read_le_float32(p + 67);

            msg.lines.push_back(line);
            offset += line_size;
        }

        handler_.onLaneLines(msg);
    }

    void ProtoParser::handleRoadObjects() {
        if (payload_buf_.size() != current_header_.payload_len) {
            ParseError err;
            err.code = ParseErrorCode::MarkingFormat;
            err.message = "Payload size mismatch for RoadObjects";
            handler_.onParseError(err);
            return;
        }
        if (payload_buf_.empty()) {
            ParseError err;
            err.code = ParseErrorCode::MarkingFormat;
            err.message = "Empty payload for RoadObjects";
            handler_.onParseError(err);
            return;
        }

        std::uint8_t num_objects = payload_buf_[0];

        // V2: Each object is 41 bytes (with pixel coordinates)
        const std::size_t object_size = 41;
        std::size_t expected_len = 1u + static_cast<std::size_t>(num_objects) * object_size;
        if (current_header_.payload_len != expected_len) {
            ParseError err;
            err.code = ParseErrorCode::MarkingFormat;
            err.message = "RoadObjects LEN mismatch: expected "
                + std::to_string(expected_len) + ", got "
                + std::to_string(current_header_.payload_len);
            handler_.onParseError(err);
            return;
        }

        RoadObjects msg;
        msg.timestamp_ms = current_header_.timestamp_ms;
        msg.seq = current_header_.seq;
        msg.objects.clear();
        msg.objects.reserve(num_objects);

        std::size_t offset = 1;

        for (std::uint8_t i = 0; i < num_objects; ++i) {
            if (offset + object_size > payload_buf_.size()) {
                ParseError err;
                err.code = ParseErrorCode::MarkingFormat;
                err.message = "Truncated RoadObject in payload";
                handler_.onParseError(err);
                return;
            }

            const std::uint8_t* p = payload_buf_.data() + offset;
            RoadObject obj;

            // V2 format: class_id(1), center_x(4), center_y(4), length(4), width(4),
            // yaw(4), center_x_px(4), center_y_px(4), width_px(4), length_px(4),
            // confidence(1), flags(1), reserved(2) = 41 bytes
            obj.class_id = static_cast<MarkingClassId>(p[0]);
            obj.center_x_m = read_le_float32(p + 1);
            obj.center_y_m = read_le_float32(p + 5);
            obj.length_m = read_le_float32(p + 9);
            obj.width_m = read_le_float32(p + 13);
            obj.yaw_rad = read_le_float32(p + 17);
            obj.center_x_px = read_le_float32(p + 21);
            obj.center_y_px = read_le_float32(p + 25);
            obj.width_px = read_le_float32(p + 29);
            obj.length_px = read_le_float32(p + 33);
            obj.confidence = p[37];
            obj.flags = p[38];
            obj.reserved = read_le_u16(p + 39);

            msg.objects.push_back(obj);
            offset += object_size;
        }

        handler_.onRoadObjects(msg);
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

                        // V2 Protocol message dispatch
                        switch (current_header_.msg_type) {
                            case MsgType::LaneLines:
                                handleLaneLines();
                                break;
                            case MsgType::RoadObjects:
                                handleRoadObjects();
                                break;
                        }
                        reset();
                    }
                    break;
            }
        }
    }
} // namespace laneproto
