#include "proto_v2_adapter.h"
#include <algorithm>
#include <cmath>

namespace laneproto {
namespace adapter {

LaneDetails ProtoV2Adapter::convertToLaneDetails(const v2::LaneLines& v2_msg) {
    LaneDetails result;
    result.timestamp_ms = v2_msg.timestamp_ms;
    result.seq = v2_msg.seq;

    // Найти левую и правую границы полосы
    const v2::LaneLine* left_line = nullptr;
    const v2::LaneLine* right_line = nullptr;

    for (const auto& line : v2_msg.lines) {
        if (line.side == LineSide::Left && !left_line) {
            left_line = &line;
        } else if (line.side == LineSide::Right && !right_line) {
            right_line = &line;
        }

        // Если нашли обе границы, можно прекратить поиск
        if (left_line && right_line) {
            break;
        }
    }

    // Заполнить детали левой границы
    if (left_line) {
        result.left.type = styleToLaneType(left_line->style);
        result.left.color = left_line->color;
        result.left.quality = 255;  // V2 не имеет quality, используем максимум
        result.left.width_m = 0.1f;  // Стандартная ширина разметки
        result.left.points_count = 3;

        // Копировать 3 контрольные точки
        result.left.points[0] = {left_line->point1_x, left_line->point1_y};
        result.left.points[1] = {left_line->point2_x, left_line->point2_y};
        result.left.points[2] = {left_line->point3_x, left_line->point3_y};

        // Вычислить left_offset из первой точки (ближайшей к камере)
        result.left_offset_m = left_line->point1_x;
    } else {
        // Нет левой границы - установить дефолтные значения
        result.left.type = LaneType::Unknown;
        result.left.color = LineColor::Unknown;
        result.left.quality = 0;
        result.left.width_m = 0.0f;
        result.left.points_count = 0;
        result.left_offset_m = -1.75f;  // Стандартная полуширина полосы
    }

    // Заполнить детали правой границы
    if (right_line) {
        result.right.type = styleToLaneType(right_line->style);
        result.right.color = right_line->color;
        result.right.quality = 255;
        result.right.width_m = 0.1f;
        result.right.points_count = 3;

        // Копировать 3 контрольные точки
        result.right.points[0] = {right_line->point1_x, right_line->point1_y};
        result.right.points[1] = {right_line->point2_x, right_line->point2_y};
        result.right.points[2] = {right_line->point3_x, right_line->point3_y};

        // Вычислить right_offset из первой точки
        result.right_offset_m = right_line->point1_x;
    } else {
        // Нет правой границы - установить дефолтные значения
        result.right.type = LaneType::Unknown;
        result.right.color = LineColor::Unknown;
        result.right.quality = 0;
        result.right.width_m = 0.0f;
        result.right.points_count = 0;
        result.right_offset_m = 1.75f;  // Стандартная полуширина полосы
    }

    // Общие параметры полосы
    result.quality = 255;              // Максимальное качество
    result.allowed_maneuvers = 0xFF;   // Все манёвры разрешены

    return result;
}

FittedLines ProtoV2Adapter::convertToFittedLines(const v2::LaneLines& v2_msg) {
    FittedLines result;
    result.timestamp_ms = v2_msg.timestamp_ms;
    result.seq = v2_msg.seq;
    result.lines.reserve(v2_msg.lines.size());

    for (const auto& v2_line : v2_msg.lines) {
        FittedLine v1_line;

        // В V2 линии не имеют class_id (это не объекты разметки)
        v1_line.class_id = MarkingClassId::Unknown;

        // Параметры линии копируются напрямую
        v1_line.side = v2_line.side;
        v1_line.color = v2_line.color;
        v1_line.style = v2_line.style;

        // Полиномиальные коэффициенты в метрах (уже в правильном формате)
        v1_line.poly_a = v2_line.poly_a;
        v1_line.poly_b = v2_line.poly_b;
        v1_line.poly_c = v2_line.poly_c;

        // Симулировать y_start/y_end из контрольных точек
        // V1 ожидает пиксельные координаты для y_start/y_end
        // Найти минимальный и максимальный Y в метрах
        float min_y = std::min({v2_line.point1_y, v2_line.point2_y, v2_line.point3_y});
        float max_y = std::max({v2_line.point1_y, v2_line.point2_y, v2_line.point3_y});

        // Преобразовать метры → пиксельные координаты
        v1_line.y_start = metersToPixelY(max_y);  // y_start - дальняя точка (больший Y)
        v1_line.y_end = metersToPixelY(min_y);    // y_end - ближняя точка (меньший Y)

        // V2 не имеет confidence/quality для линий
        v1_line.confidence = 255;  // Максимальная уверенность
        v1_line.quality = 255;     // Максимальное качество

        result.lines.push_back(v1_line);
    }

    return result;
}

MarkingObjects ProtoV2Adapter::convertToMarkingObjects(const v2::RoadObjects& v2_msg) {
    MarkingObjects result;
    result.timestamp_ms = v2_msg.timestamp_ms;
    result.seq = v2_msg.seq;
    result.objects.reserve(v2_msg.objects.size());

    for (const auto& v2_obj : v2_msg.objects) {
        MarkingObject v1_obj;

        // class_id копируется напрямую (enum совместимы)
        v1_obj.class_id = v2_obj.class_id;

        // Координаты и размеры в метрах (напрямую)
        v1_obj.x_m = v2_obj.center_x;
        v1_obj.y_m = v2_obj.center_y;
        v1_obj.length_m = v2_obj.length;
        v1_obj.width_m = v2_obj.width;

        // КРИТИЧНО: конвертация yaw из РАДИАН в ГРАДУСЫ!
        // V2 использует радианы, V1 и QML используют градусы
        v1_obj.yaw_deg = radiansToDegrees(v2_obj.yaw);

        // Confidence и flags копируются напрямую
        v1_obj.confidence = v2_obj.confidence;
        v1_obj.flags = v2_obj.flags;

        // Определить цвет и стиль линии из class_id
        // V2 не передаёт line_color/line_style для объектов,
        // но можем вывести из class_id
        switch (v1_obj.class_id) {
            case MarkingClassId::SolidSingleWhite:
            case MarkingClassId::DoubleWhite:
            case MarkingClassId::DashedWhite:
                v1_obj.line_color = LineColor::White;
                break;

            case MarkingClassId::SolidSingleYellow:
            case MarkingClassId::DoubleYellow:
            case MarkingClassId::DashedYellow:
                v1_obj.line_color = LineColor::Yellow;
                break;

            case MarkingClassId::SolidSingleRed:
                v1_obj.line_color = LineColor::Red;
                break;

            default:
                v1_obj.line_color = LineColor::White;  // Дефолт для объектов
                break;
        }

        // Определить стиль
        switch (v1_obj.class_id) {
            case MarkingClassId::SolidSingleWhite:
            case MarkingClassId::SolidSingleYellow:
            case MarkingClassId::SolidSingleRed:
                v1_obj.line_style = LineStyle::Solid;
                break;

            case MarkingClassId::DoubleWhite:
            case MarkingClassId::DoubleYellow:
                v1_obj.line_style = LineStyle::Double;
                break;

            case MarkingClassId::DashedWhite:
            case MarkingClassId::DashedYellow:
                v1_obj.line_style = LineStyle::Dashed;
                break;

            default:
                v1_obj.line_style = LineStyle::Unknown;
                break;
        }

        result.objects.push_back(v1_obj);
    }

    return result;
}

// Private методы

std::int16_t ProtoV2Adapter::metersToPixelY(float y_meters) {
    // Обратная функция от RoadCanvas2D.qml:
    // worldY_m = ((VIDEO_HEIGHT - y_px) / VIDEO_HEIGHT) * VIEW_RANGE_METERS
    //
    // Решаем для y_px:
    // y_px = VIDEO_HEIGHT - (worldY_m * VIDEO_HEIGHT / VIEW_RANGE_METERS)

    float y_px = VIDEO_HEIGHT - (y_meters * VIDEO_HEIGHT / VIEW_RANGE_METERS);

    // Ограничить диапазон [0, VIDEO_HEIGHT]
    y_px = std::max(0.0f, std::min(VIDEO_HEIGHT, y_px));

    return static_cast<std::int16_t>(y_px);
}

LaneType ProtoV2Adapter::styleToLaneType(LineStyle style) {
    // Попытка восстановить LaneType из LineStyle
    // Это приблизительный маппинг, так как информация частично потеряна в V2

    switch (style) {
        case LineStyle::Solid:
            return LaneType::Solid;

        case LineStyle::Dashed:
            return LaneType::Dashed;

        case LineStyle::Double:
            return LaneType::DoubleSolid;

        case LineStyle::Unknown:
        default:
            return LaneType::Unknown;
    }
}

} // namespace adapter
} // namespace laneproto
