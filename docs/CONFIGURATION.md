# Конфигурация приложения

## Обзор

Приложение использует JSON файлы для конфигурации. Основные файлы:

| Файл | Назначение |
|------|------------|
| `config.json` | Основная конфигурация (сеть, видео, предупреждения) |
| `scene_config.json` | Параметры 3D сцены |

## config.json

### Полная структура

```json
{
  "network": {
    "host": "127.0.0.1",
    "port": 9000,
    "auto_reconnect": true,
    "reconnect_interval_ms": 5000
  },
  "video": {
    "source_url": "udp://239.0.0.1:5000",
    "auto_start": false
  },
  "video_processing": {
    "gst_bus_poll_interval_ms": 250,
    "rtp_jitter_buffer_latency_ms": 50,
    "overlay": {
      "fitted_line_points_count": 100,
      "fitted_line_source_width": 640.0,
      "fitted_line_source_height": 480.0
    }
  },
  "warning": {
    "lane_departure_threshold_m": 0.3,
    "crosswalk_distance_threshold_m": 30.0,
    "crosswalk_critical_distance_m": 10.0,
    "enable_crosswalk_warnings": true,
    "enable_lane_departure_warnings": true
  },
  "sync": {
    "max_timestamp_diff_ms": 500,
    "enable_sync_monitoring": true
  },
  "ui": {
    "main_window_width": 1280,
    "main_window_height": 720
  }
}
```

### Секция network

Настройки TCP соединения для получения данных разметки.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `host` | string | `"127.0.0.1"` | IP адрес TCP сервера |
| `port` | int | `9000` | Порт TCP сервера |
| `auto_reconnect` | bool | `true` | Автоматическое переподключение |
| `reconnect_interval_ms` | int | `5000` | Интервал переподключения (мс) |

**Примеры:**

```json
// Локальный сервер
"network": {
  "host": "127.0.0.1",
  "port": 9000
}

// Удалённый сервер без автопереподключения
"network": {
  "host": "192.168.1.100",
  "port": 9000,
  "auto_reconnect": false
}
```

### Секция video

Настройки видеопотока.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `source_url` | string | `""` | URL видеопотока |
| `auto_start` | bool | `false` | Автозапуск видео |

**Поддерживаемые форматы URL:**

```json
// UDP multicast
"source_url": "udp://239.0.0.1:5000"

// RTSP
"source_url": "rtsp://192.168.1.100:8554/stream"

// RTP
"source_url": "rtp://0.0.0.0:5000"

// Локальный файл
"source_url": "file:///path/to/video.mp4"

// Тестовый источник GStreamer
"source_url": "videotestsrc"
```

### Секция video_processing

Параметры обработки видео и оверлея.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `gst_bus_poll_interval_ms` | int | `250` | Интервал опроса GStreamer bus |
| `rtp_jitter_buffer_latency_ms` | int | `50` | Задержка jitter buffer для RTP |

**Подсекция overlay:**

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `fitted_line_points_count` | int | `100` | Количество точек для полиномиальных линий |
| `fitted_line_source_width` | float | `640.0` | Ширина исходного изображения |
| `fitted_line_source_height` | float | `480.0` | Высота исходного изображения |

### Секция warning

Настройки системы предупреждений.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `lane_departure_threshold_m` | float | `0.3` | Порог схода с полосы (м) |
| `crosswalk_distance_threshold_m` | float | `30.0` | Расстояние предупреждения о переходе (м) |
| `crosswalk_critical_distance_m` | float | `10.0` | Критическое расстояние до перехода (м) |
| `enable_crosswalk_warnings` | bool | `true` | Включить предупреждения о переходах |
| `enable_lane_departure_warnings` | bool | `true` | Включить предупреждения о сходе с полосы |

**Логика предупреждений:**

```
distance > crosswalk_distance_threshold_m     → Нет предупреждения
distance <= crosswalk_distance_threshold_m    → Warning (жёлтый)
distance <= crosswalk_critical_distance_m     → Critical (красный)

|center_offset| > lane_departure_threshold_m  → Lane Departure Warning
```

### Секция sync

Настройки синхронизации между данными и видео.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `max_timestamp_diff_ms` | int | `500` | Максимальная разница timestamp |
| `enable_sync_monitoring` | bool | `true` | Включить мониторинг синхронизации |

### Секция ui

Настройки пользовательского интерфейса.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `main_window_width` | int | `1280` | Ширина главного окна |
| `main_window_height` | int | `720` | Высота главного окна |

---

## scene_config.json

Параметры 3D визуализации.

### Полная структура

```json
{
  "scale": {
    "scale_factor": 100
  },
  "camera": {
    "height": 1000.0,
    "distance": -1000.0,
    "pitch_angle_deg": -20.0,
    "clip_near": 10.1,
    "clip_far": 50000.0
  },
  "lighting": {
    "main_light_brightness": 0.8,
    "secondary_light_brightness": 0.5,
    "tertiary_light_brightness": 0.3,
    "point_light_brightness": 5.0
  },
  "road": {
    "width_m": 12.0,
    "length_m": 100.0,
    "thickness_m": 0.01,
    "edge_line_width": 0.15,
    "center_line_width": 0.1
  },
  "marking": {
    "y_position_above_plane": 0.02,
    "min_width_m": 0.05,
    "height_m": 0.03,
    "min_opacity": 0.3,
    "max_opacity": 1.0,
    "confidence_divisor": 100.0
  },
  "car": {
    "y_elevation": 0.75,
    "marker_scale": 0.08
  },
  "center_offset": {
    "width": 320,
    "height": 130,
    "max_offset_m": 1.0,
    "safe_threshold_m": 0.3,
    "critical_threshold_m": 0.6
  },
  "dashboard": {
    "width": 800,
    "height": 600
  },
  "warning_panel": {
    "width": 360,
    "color_animation_duration_ms": 150
  }
}
```

### Секция scale

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `scale_factor` | int | `100` | Масштаб сцены (1м = scale_factor единиц) |

### Секция camera

Параметры изометрической камеры.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `height` | float | `1000.0` | Высота камеры над сценой |
| `distance` | float | `-1000.0` | Расстояние камеры (отрицательное = сзади) |
| `pitch_angle_deg` | float | `-20.0` | Угол наклона камеры (градусы) |
| `clip_near` | float | `10.1` | Ближняя плоскость отсечения |
| `clip_far` | float | `50000.0` | Дальняя плоскость отсечения |

**Визуализация камеры:**

```
        ↑ Y (высота)
        │
        │    Camera
        │   ╱
        │  ╱ pitch_angle_deg
        │ ╱
────────┼──────────→ X
        │
        ▼ Z (distance)
```

### Секция lighting

Настройки освещения 3D сцены.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `main_light_brightness` | float | `0.8` | Яркость основного света |
| `secondary_light_brightness` | float | `0.5` | Яркость вторичного света |
| `tertiary_light_brightness` | float | `0.3` | Яркость третичного света |
| `point_light_brightness` | float | `5.0` | Яркость точечного света |

### Секция road

Параметры дорожного полотна.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `width_m` | float | `12.0` | Ширина дороги (м) |
| `length_m` | float | `100.0` | Длина видимой дороги (м) |
| `thickness_m` | float | `0.01` | Толщина плоскости дороги |
| `edge_line_width` | float | `0.15` | Ширина краевых линий (м) |
| `center_line_width` | float | `0.1` | Ширина центральной линии (м) |

### Секция marking

Параметры визуализации объектов разметки.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `y_position_above_plane` | float | `0.02` | Высота над поверхностью дороги |
| `min_width_m` | float | `0.05` | Минимальная ширина объекта (м) |
| `height_m` | float | `0.03` | Высота 3D объекта (м) |
| `min_opacity` | float | `0.3` | Минимальная прозрачность |
| `max_opacity` | float | `1.0` | Максимальная прозрачность |
| `confidence_divisor` | float | `100.0` | Делитель для confidence |

**Расчёт прозрачности:**

```
opacity = min_opacity + (confidence / confidence_divisor) * (max_opacity - min_opacity)
```

### Секция car

Параметры 3D модели автомобиля.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `y_elevation` | float | `0.75` | Высота модели над дорогой |
| `marker_scale` | float | `0.08` | Масштаб маркера машины |

### Секция center_offset

Параметры индикатора смещения от центра.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `width` | int | `320` | Ширина индикатора (px) |
| `height` | int | `130` | Высота индикатора (px) |
| `max_offset_m` | float | `1.0` | Максимальное отображаемое смещение |
| `safe_threshold_m` | float | `0.3` | Порог безопасной зоны (зелёный) |
| `critical_threshold_m` | float | `0.6` | Порог критической зоны (красный) |

**Цветовая схема индикатора:**

```
|offset| <= safe_threshold     → Зелёный
safe_threshold < |offset| <= critical_threshold → Жёлтый
|offset| > critical_threshold  → Красный
```

### Секция dashboard

Размеры 3D панели.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `width` | int | `800` | Ширина 3D виджета |
| `height` | int | `600` | Высота 3D виджета |

### Секция warning_panel

Параметры панели предупреждений.

| Параметр | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `width` | int | `360` | Ширина панели |
| `color_animation_duration_ms` | int | `150` | Длительность анимации цвета |

---

## Примеры конфигураций

### Минимальная конфигурация

```json
{
  "network": {
    "host": "127.0.0.1",
    "port": 9000
  }
}
```

### Конфигурация для тестирования

```json
{
  "network": {
    "host": "127.0.0.1",
    "port": 9000,
    "auto_reconnect": false
  },
  "video": {
    "source_url": "videotestsrc",
    "auto_start": true
  },
  "warning": {
    "enable_crosswalk_warnings": true,
    "enable_lane_departure_warnings": true
  }
}
```

### Конфигурация для RTSP камеры

```json
{
  "network": {
    "host": "192.168.1.50",
    "port": 9000,
    "auto_reconnect": true,
    "reconnect_interval_ms": 3000
  },
  "video": {
    "source_url": "rtsp://admin:password@192.168.1.100:554/stream1",
    "auto_start": true
  },
  "video_processing": {
    "rtp_jitter_buffer_latency_ms": 100
  }
}
```

### Конфигурация для высокого разрешения

```json
{
  "video_processing": {
    "overlay": {
      "fitted_line_points_count": 200,
      "fitted_line_source_width": 1920.0,
      "fitted_line_source_height": 1080.0
    }
  },
  "ui": {
    "main_window_width": 1920,
    "main_window_height": 1080
  }
}
```

---

## Командная строка

Приложение поддерживает аргументы командной строки:

```bash
./Dashboard-for-road-marking [OPTIONS]

OPTIONS:
  --config <path>     Путь к файлу конфигурации (default: config.json)
  --scene <path>      Путь к конфигурации сцены (default: scene_config.json)
  --host <ip>         Переопределить хост TCP
  --port <port>       Переопределить порт TCP
  --video <url>       Переопределить URL видео
  --help              Показать справку
```

**Примеры:**

```bash
# Использовать кастомную конфигурацию
./Dashboard-for-road-marking --config /etc/dashboard/config.json

# Переопределить хост и порт
./Dashboard-for-road-marking --host 192.168.1.100 --port 8080

# Указать видеопоток
./Dashboard-for-road-marking --video "rtsp://camera.local/stream"
```

---

## Переменные окружения

| Переменная | Описание |
|------------|----------|
| `DASHBOARD_CONFIG` | Путь к файлу конфигурации |
| `DASHBOARD_LOG_LEVEL` | Уровень логирования (DEBUG, INFO, WARN, ERROR) |
| `QT_QPA_PLATFORM` | Платформа Qt (xcb, wayland, offscreen) |
| `QSG_RHI_BACKEND` | Backend для Qt Quick (opengl, vulkan, metal) |

**Пример:**

```bash
export DASHBOARD_LOG_LEVEL=DEBUG
export QSG_RHI_BACKEND=opengl
./Dashboard-for-road-marking
```

---

## Валидация конфигурации

При загрузке конфигурации выполняется валидация:

1. **Проверка JSON синтаксиса**
2. **Проверка типов значений**
3. **Проверка допустимых диапазонов**
4. **Применение значений по умолчанию для отсутствующих параметров**

**Пример ошибки:**

```
[ERROR] Configuration error: network.port must be between 1 and 65535
[WARN] Using default value for video.source_url: ""
```
