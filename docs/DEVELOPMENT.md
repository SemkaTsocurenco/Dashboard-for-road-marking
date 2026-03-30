# Руководство для разработчиков

## Настройка среды разработки

### Рекомендуемые IDE

| IDE | Настройка |
|-----|-----------|
| **Qt Creator** | Открыть CMakeLists.txt как проект |
| **VS Code** | Установить расширения C/C++, CMake Tools, QML |
| **CLion** | Открыть как CMake проект |

### VS Code конфигурация

**`.vscode/settings.json`:**
```json
{
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "files.associations": {
        "*.qml": "qml"
    }
}
```

**`.vscode/c_cpp_properties.json`:**
```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/include/qt6/**",
                "/usr/include/gstreamer-1.0/**"
            ],
            "defines": ["QT_QML_DEBUG"],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ]
}
```

### Генерация compile_commands.json

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
ln -s build/compile_commands.json .
```

---

## Структура кода

### Соглашения об именовании

| Элемент | Стиль | Пример |
|---------|-------|--------|
| Классы | PascalCase | `ConnectionManager` |
| Методы | camelCase | `connectToHost()` |
| Переменные | snake_case | `center_offset_m` |
| Члены класса | m_ prefix | `m_socket` |
| Константы | UPPER_SNAKE | `MAX_BUFFER_SIZE` |
| Файлы C++ | PascalCase.cpp/hpp | `ConnectionManager.cpp` |
| Файлы QML | PascalCase.qml | `WarningPanel.qml` |

### Структура заголовочного файла

```cpp
#ifndef PROJECT_MODULE_CLASSNAME_H
#define PROJECT_MODULE_CLASSNAME_H

#include <QtCore/QObject>
#include <memory>

// Forward declarations
namespace domain {
class LaneState;
}

namespace module {

/**
 * @brief Краткое описание класса
 *
 * Подробное описание функциональности.
 */
class ClassName : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit ClassName(QObject* parent = nullptr);
    ~ClassName() override;

    // Геттеры
    bool isConnected() const;

    // Публичные методы
    Q_INVOKABLE void doSomething();

signals:
    void connectedChanged(bool connected);

public slots:
    void onDataReceived();

private:
    void privateHelper();

    // Члены класса
    bool m_connected = false;
    std::unique_ptr<SomeClass> m_impl;
};

} // namespace module

#endif // PROJECT_MODULE_CLASSNAME_H
```

---

## Добавление новой функциональности

### Добавление нового типа объекта разметки

**1. Обновить enum в `domain/MarkingObject.h`:**

```cpp
enum class ClassId {
    // ... существующие
    NewObjectType = 0x20  // Новый тип
};
```

**2. Обновить метод `className()`:**

```cpp
QString MarkingObject::className() const {
    switch (class_id) {
        // ... существующие
        case ClassId::NewObjectType: return "New Object";
        default: return "Unknown";
    }
}
```

**3. Обновить адаптер `parser/proto_v2_adapter.cpp`:**

```cpp
domain::MarkingObject::ClassId toClassId(uint8_t id) {
    switch (id) {
        // ... существующие
        case 0x20: return domain::MarkingObject::ClassId::NewObjectType;
        default: return domain::MarkingObject::ClassId::Unknown;
    }
}
```

**4. Добавить визуализацию в `MarkingOverlayProcessor`:**

```cpp
void MarkingOverlayProcessor::drawMarkingObjects(QPainter& painter, const QSize& size) {
    for (const auto& obj : m_objects) {
        switch (obj.class_id) {
            // ... существующие
            case domain::MarkingObject::ClassId::NewObjectType:
                drawNewObjectType(painter, obj, size);
                break;
        }
    }
}
```

**5. Добавить 3D компонент в QML (опционально):**

```qml
// NewObjectType3D.qml
import QtQuick3D

Model {
    property real xPosition
    property real yPosition
    // ...
}
```

### Добавление нового типа предупреждения

**1. Обновить enum в `domain/Warning.h`:**

```cpp
enum class Type {
    // ... существующие
    NewWarningType
};
```

**2. Обновить `WarningEngine`:**

```cpp
std::vector<Warning> WarningEngine::update(...) const {
    std::vector<Warning> warnings;

    // ... существующие проверки

    // Новая проверка
    if (auto warning = checkNewCondition(lane, markings)) {
        warnings.push_back(*warning);
    }

    return warnings;
}

std::optional<Warning> WarningEngine::checkNewCondition(...) const {
    if (/* условие */) {
        Warning w;
        w.type = Warning::Type::NewWarningType;
        w.severity = Warning::Severity::Warning;
        w.message = "Описание предупреждения";
        return w;
    }
    return std::nullopt;
}
```

**3. Обновить `WarningListModel` (роли, если нужно):**

```cpp
QVariant WarningListModel::data(const QModelIndex& index, int role) const {
    // ... добавить обработку новых ролей
}
```

**4. Обновить `WarningPanel.qml` (если нужен специальный вид):**

```qml
delegate: WarningItem {
    // Специальная обработка для нового типа
    specialIcon: model.type === Warning.NewWarningType ? "new_icon.png" : ""
}
```

---

## Тестирование

### Запуск тестов

```bash
cd build
ctest --output-on-failure
```

### Написание тестов

**Пример unit теста для ProtoParser:**

```cpp
// tests/test_proto_parser.cpp
#include <gtest/gtest.h>
#include "parser/proto_parser.h"

class MockHandler : public ProtoParser::IMessageHandler {
public:
    void onLaneLines(const laneproto::LaneLines& msg) override {
        lane_lines_received = true;
        last_lane_lines = msg;
    }

    void onRoadObjects(const laneproto::RoadObjects& msg) override {
        road_objects_received = true;
    }

    bool lane_lines_received = false;
    bool road_objects_received = false;
    laneproto::LaneLines last_lane_lines;
};

TEST(ProtoParserTest, ParseValidLaneLines) {
    ProtoParser parser;
    MockHandler handler;
    parser.setHandler(&handler);

    // Валидный кадр LaneLines
    std::vector<uint8_t> frame = {
        0xAA,                           // Sync
        0x02, 0x01, 0x00,               // Version, Type, Seq
        0x00, 0x00, 0x00, 0x00,         // Timestamp
        0x06, 0x00,                     // Payload length
        // ... payload
        0x00, 0x00                      // CRC (нужно рассчитать)
    };

    parser.feed(frame.data(), frame.size());

    EXPECT_TRUE(handler.lane_lines_received);
}

TEST(ProtoParserTest, RejectInvalidCrc) {
    ProtoParser parser;
    MockHandler handler;
    parser.setHandler(&handler);

    std::vector<uint8_t> frame = { /* кадр с неверным CRC */ };
    parser.feed(frame.data(), frame.size());

    EXPECT_FALSE(handler.lane_lines_received);
    EXPECT_EQ(parser.framesDropped(), 1);
}
```

### Тестовый TCP сервер

```python
#!/usr/bin/env python3
# tools/test_server.py

import socket
import struct
import time

def create_lane_lines_frame():
    """Создаёт тестовый кадр LaneLines"""
    # Header
    version = 0x02
    msg_type = 0x01
    sequence = 0
    timestamp = int(time.time() * 1000) & 0xFFFFFFFF

    # Payload: 2 линии
    line_count = 2
    payload = struct.pack('<B', line_count)

    for i in range(line_count):
        line_id = i
        style = 1  # Solid
        color = 1  # White
        offset = -1.5 + i * 3.0
        confidence = 95.0
        poly_a, poly_b, poly_c = 0.0, 0.0, offset

        payload += struct.pack('<BBBffff',
            line_id, style, color,
            offset, confidence,
            poly_a, poly_b, poly_c)

    # Center offset и quality
    payload += struct.pack('<fB', 0.1, 90)

    payload_len = len(payload)

    # Header bytes
    header = struct.pack('<BBBI H',
        version, msg_type, sequence, timestamp, payload_len)

    # CRC (упрощённо)
    crc = 0x0000  # В реальности нужно рассчитать

    # Полный кадр
    frame = bytes([0xAA]) + header + payload + struct.pack('<H', crc)
    return frame

def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(('0.0.0.0', 9000))
    server.listen(1)

    print("Test server listening on :9000")

    while True:
        conn, addr = server.accept()
        print(f"Client connected: {addr}")

        try:
            while True:
                frame = create_lane_lines_frame()
                conn.send(frame)
                time.sleep(0.033)  # ~30 FPS
        except BrokenPipeError:
            print("Client disconnected")

if __name__ == '__main__':
    main()
```

---

## Отладка

### QML отладка

**Включение QML debugger:**

```bash
QML_DEBUG_ARGUMENTS="-qmljsdebugger=port:1234,block" ./Dashboard-for-road-marking
```

**Отладочный вывод в QML:**

```qml
Component.onCompleted: {
    console.log("LaneViewModel valid:", laneViewModel.valid)
    console.log("Center offset:", laneViewModel.centerOffset)
}

onCenterOffsetChanged: {
    console.log("Offset changed to:", laneViewModel.centerOffset)
}
```

### C++ отладка

**Включение подробного логирования:**

```cpp
// В main.cpp
qSetMessagePattern("[%{type}] %{file}:%{line} - %{message}");
```

**Использование логгера:**

```cpp
#include "logger/LoggerMacros.hpp"

void ConnectionManager::onDataReceived() {
    LOG_DEBUG("Received {} bytes", data.size());

    if (error) {
        LOG_ERROR("Failed to process: {}", error.message());
    }
}
```

### GStreamer отладка

```bash
# Подробный вывод GStreamer
GST_DEBUG=3 ./Dashboard-for-road-marking

# Отладка конкретного элемента
GST_DEBUG=udpsrc:5,rtpjitterbuffer:4 ./Dashboard-for-road-marking

# Создание dot-файла pipeline
GST_DEBUG_DUMP_DOT_DIR=/tmp ./Dashboard-for-road-marking
dot -Tpng /tmp/pipeline.dot -o pipeline.png
```

### Network отладка

```bash
# Мониторинг TCP трафика
sudo tcpdump -i lo port 9000 -X

# Wireshark для анализа протокола
wireshark -i lo -f "port 9000"
```

---

## Производительность

### Профилирование

```bash
# Valgrind (callgrind)
valgrind --tool=callgrind ./Dashboard-for-road-marking
kcachegrind callgrind.out.*

# perf
perf record -g ./Dashboard-for-road-marking
perf report

# Qt Creator Profiler
# Analyze → QML Profiler
```

### Оптимизации

**1. Избегать лишних копий:**

```cpp
// Плохо
void updateData(std::vector<Object> objects);

// Хорошо
void updateData(const std::vector<Object>& objects);
void updateData(std::vector<Object>&& objects);
```

**2. Использовать move semantics:**

```cpp
void ConnectionManager::onLaneLines(laneproto::LaneLines lines) {
    m_laneState = proto_v2_adapter::toLaneState(std::move(lines));
}
```

**3. Batch обновления в моделях:**

```cpp
void MarkingObjectListModel::updateFromDomain(const domain::MarkingObjectModel& model) {
    beginResetModel();
    m_objects = model.objects();
    endResetModel();
    emit countChanged();
}
```

**4. Ленивое обновление QML:**

```qml
// Использовать Loader для тяжёлых компонентов
Loader {
    active: visible && laneViewModel.valid
    sourceComponent: heavyComponent
}
```

---

## Code Review Checklist

### Общее

- [ ] Код компилируется без warnings
- [ ] Следует соглашениям об именовании
- [ ] Нет утечек памяти (использовать smart pointers)
- [ ] Потокобезопасность (mutex для shared data)

### Qt/QML

- [ ] Q_PROPERTY имеет NOTIFY сигнал
- [ ] Q_INVOKABLE для методов, вызываемых из QML
- [ ] Сигналы подключены с правильным ConnectionType
- [ ] Нет блокирующих операций в main thread

### Производительность

- [ ] Нет лишних копий больших объектов
- [ ] Используется const& для read-only параметров
- [ ] Batch обновления вместо множественных emit

---

## CI/CD

### GitHub Actions пример

```yaml
# .github/workflows/build.yml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-22.04

    steps:
    - uses: actions/checkout@v3

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          qt6-base-dev qt6-declarative-dev qt6-multimedia-dev qt6-quick3d-dev \
          libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

    - name: Configure
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release

    - name: Build
      run: cmake --build build -j$(nproc)

    - name: Test
      run: cd build && ctest --output-on-failure
```

---

## Часто задаваемые вопросы

### QML не видит ViewModel

**Проблема:** QML показывает undefined для свойств ViewModel.

**Решение:**
1. Проверить регистрацию в QML engine
2. Проверить setContextProperty до загрузки QML
3. Убедиться что Q_PROPERTY корректно определены

```cpp
// Правильный порядок
engine.rootContext()->setContextProperty("viewModel", viewModel);
engine.load(QUrl("qrc:/qml/main.qml"));  // После setContextProperty!
```

### Видео не отображается

**Проблема:** Чёрный экран вместо видео.

**Решение:**
1. Проверить GStreamer pipeline:
   ```bash
   gst-launch-1.0 udpsrc port=5000 ! ... ! autovideosink
   ```
2. Проверить firewall для UDP портов
3. Убедиться что кодеки установлены

### TCP данные не приходят

**Проблема:** ConnectionManager не получает данные.

**Решение:**
1. Проверить подключение: `netcat -l 9000`
2. Проверить формат данных (sync byte 0xAA)
3. Включить логирование в TcpReaderWorker

### 3D сцена не рендерится

**Проблема:** Пустой View3D или ошибки OpenGL.

**Решение:**
1. Проверить OpenGL: `glxinfo | grep "OpenGL version"`
2. Попробовать software rendering: `QSG_RHI_BACKEND=opengl`
3. Обновить драйверы видеокарты
