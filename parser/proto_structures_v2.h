#pragma once
#include <cstdint>
#include <vector>
#include "proto_parser.h"  // для TimestampMs, SequenceNumber, базовых enum'ов

namespace laneproto {
namespace v2 {

/**
 * V2 Protocol Message Types
 *
 * V2 упрощает протокол до двух типов сообщений:
 * - 0x01 LANE_LINES: линии разметки с полиномами и точками
 * - 0x02 ROAD_OBJECTS: объекты на дороге
 */
enum class MsgTypeV2 : std::uint8_t {
    LaneLines    = 0x01,  // Линии дорожной разметки (38 байт на линию)
    RoadObjects  = 0x02,  // Объекты разметки (25 байт на объект)
};

/**
 * V2 Lane Line Structure (38 bytes per line)
 *
 * Объединяет информацию из V1 LaneDetails + FittedLines:
 * - Параметры линии (side, style, color)
 * - Полиномиальные коэффициенты для вычисления траектории
 * - 3 контрольные точки в метрах
 *
 * Структура payload:
 * [side(1B)][style(1B)][color(1B)]
 * [poly_a(4B)][poly_b(4B)][poly_c(4B)]
 * [point1_x(4B)][point1_y(4B)]
 * [point2_x(4B)][point2_y(4B)]
 * [point3_x(4B)][point3_y(4B)]
 */
struct LaneLine {
    // Параметры линии (повторное использование V1 enum'ов)
    laneproto::LineSide  side   = laneproto::LineSide::Unknown;   // Сторона: left/right/center
    laneproto::LineStyle style  = laneproto::LineStyle::Unknown;  // Стиль: solid/dashed/double
    laneproto::LineColor color  = laneproto::LineColor::Unknown;  // Цвет: white/yellow/red

    // Полиномиальные коэффициенты: x = a*y² + b*y + c
    // Все координаты в метрах
    float poly_a = 0.0f;
    float poly_b = 0.0f;
    float poly_c = 0.0f;

    // Три контрольные точки в метрах (мировая система координат)
    // point1 - ближайшая (меньший Y)
    // point2 - средняя
    // point3 - дальняя (больший Y)
    float point1_x = 0.0f;
    float point1_y = 0.0f;
    float point2_x = 0.0f;
    float point2_y = 0.0f;
    float point3_x = 0.0f;
    float point3_y = 0.0f;
};

/**
 * V2 LANE_LINES Message (MSG_TYPE = 0x01)
 *
 * Payload: [count(1B)] + N * LaneLine(38B)
 * Total frame size: 12 + N*38 bytes
 */
struct LaneLines {
    TimestampMs timestamp_ms{};     // Из заголовка
    SequenceNumber seq{};           // Из заголовка
    std::vector<LaneLine> lines;    // Массив линий
};

/**
 * V2 Road Object Structure (25 bytes per object)
 *
 * Объединяет V1 MarkingObjects + MarkingObjectsEx:
 * - Все координаты в метрах (float32)
 * - yaw в РАДИАНАХ (ВАЖНО: V1 использовал градусы!)
 *
 * Структура payload:
 * [class_id(1B)]
 * [center_x(4B)][center_y(4B)]
 * [length(4B)][width(4B)]
 * [yaw(4B)]
 * [confidence(1B)][flags(1B)]
 * [reserved(2B)]
 */
struct RoadObject {
    laneproto::MarkingClassId class_id = laneproto::MarkingClassId::Unknown;

    // Позиция центра объекта в метрах
    // Система координат: X вправо, Y вперёд, начало в камере
    float center_x = 0.0f;  // метры
    float center_y = 0.0f;  // метры

    // Размеры объекта в метрах
    float length = 0.0f;    // метры
    float width = 0.0f;     // метры

    // Ориентация в РАДИАНАХ (не градусы!)
    // 0 = объект направлен вперёд
    // π/2 = объект направлен вправо
    float yaw = 0.0f;       // радианы

    // Уверенность детектирования (0-255)
    std::uint8_t confidence = 0;

    // Битовые флаги состояния
    std::uint8_t flags = 0;

    // Резерв для будущего использования
    std::uint16_t reserved = 0;
};

/**
 * V2 ROAD_OBJECTS Message (MSG_TYPE = 0x02)
 *
 * Payload: [count(1B)] + N * RoadObject(25B)
 * Total frame size: 12 + N*25 bytes
 */
struct RoadObjects {
    TimestampMs timestamp_ms{};        // Из заголовка
    SequenceNumber seq{};              // Из заголовка
    std::vector<RoadObject> objects;   // Массив объектов
};

} // namespace v2
} // namespace laneproto
