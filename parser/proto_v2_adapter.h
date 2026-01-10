#pragma once
#include "proto_parser.h"
#include "proto_structures_v2.h"
#include <cmath>

namespace laneproto {
namespace adapter {

/**
 * V2→V1 Protocol Adapter
 *
 * Преобразует V2 протокол в V1 структуры для совместимости
 * с существующими domain моделями.
 *
 * Ключевые преобразования:
 * 1. V2 LANE_LINES → V1 LaneDetails + V1 FittedLines
 * 2. V2 ROAD_OBJECTS → V1 MarkingObjects
 * 3. Радианы → Градусы (для yaw)
 * 4. Метры → Пиксели (для y_start/y_end в FittedLines)
 */
class ProtoV2Adapter {
public:
    ProtoV2Adapter() = default;
    ~ProtoV2Adapter() = default;

    /**
     * Конвертирует V2 LANE_LINES в V1 LaneDetails
     *
     * Извлекает из v2::LaneLines:
     * - Левую и правую границы полосы
     * - 3 контрольные точки для каждой границы
     * - Параметры линий (color, style)
     * - Вычисляет offsets из point1_x
     *
     * @param v2_msg V2 сообщение с линиями
     * @return V1 LaneDetails структура
     */
    static LaneDetails convertToLaneDetails(const v2::LaneLines& v2_msg);

    /**
     * Конвертирует V2 LANE_LINES в V1 FittedLines
     *
     * Преобразует полиномиальные линии:
     * - Копирует poly_a, poly_b, poly_c напрямую
     * - Симулирует y_start/y_end из контрольных точек
     * - Преобразует метры → пиксельные координаты
     *
     * @param v2_msg V2 сообщение с линиями
     * @return V1 FittedLines структура
     */
    static FittedLines convertToFittedLines(const v2::LaneLines& v2_msg);

    /**
     * Конвертирует V2 ROAD_OBJECTS в V1 MarkingObjects
     *
     * ВАЖНО: Конвертирует yaw из радиан в градусы!
     * Все остальные поля копируются напрямую.
     *
     * @param v2_msg V2 сообщение с объектами
     * @return V1 MarkingObjects структура
     */
    static MarkingObjects convertToMarkingObjects(const v2::RoadObjects& v2_msg);

private:
    /**
     * Конвертация радиан → градусы
     *
     * @param radians Угол в радианах
     * @return Угол в градусах
     */
    static inline float radiansToDegrees(float radians) {
        return radians * 180.0f / static_cast<float>(M_PI);
    }

    /**
     * Преобразование метров → пиксельные координаты
     *
     * V1 FittedLines ожидает y_start/y_end в пиксельных координатах видео.
     * Симулируем обратное преобразование от RoadCanvas2D.qml
     *
     * Формула из QML: worldY_m = ((videoHeight - y_px) / videoHeight) * viewRange
     * Обратная: y_px = videoHeight - (worldY_m * videoHeight / viewRange)
     *
     * @param y_meters Y координата в метрах
     * @return Y координата в пикселях
     */
    static std::int16_t metersToPixelY(float y_meters);

    /**
     * Преобразование StyleEnum в LaneType
     *
     * Попытка восстановить LaneType из LineStyle:
     * - Solid → Solid
     * - Dashed → Dashed
     * - Double → DoubleSolid
     *
     * @param style LineStyle из V2
     * @return LaneType для V1
     */
    static LaneType styleToLaneType(LineStyle style);

    // Константы для преобразования координат
    // Эти значения должны соответствовать RoadCanvas2D.qml
    static constexpr float VIDEO_HEIGHT = 1080.0f;      // Высота видео в пикселях
    static constexpr float VIDEO_WIDTH = 1920.0f;       // Ширина видео в пикселях
    static constexpr float VIEW_RANGE_METERS = 40.0f;   // Диапазон видимости вперёд (из QML)
};

} // namespace adapter
} // namespace laneproto
