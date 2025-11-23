# Dashboard UI Architecture - Hybrid Qt Widgets + Qt Quick 3D

## Обзор

Гибридный UI, сочетающий:
- **Слева (50%)**: Видеопоток (Qt Widgets - NetworkVideoWidget)
- **Справа (50%)**: 3D приборная панель автомобиля (Qt Quick 3D)

## Архитектура компонентов

### Общая структура окна

```
MainWindow (QMainWindow)
│
├── [Top] Control Panel (QGroupBox)
│   └── Host/Port inputs, Connect/Disconnect buttons
│
├── [Middle] QSplitter (horizontal, 50/50)
│   ├── Left: NetworkVideoWidget (существующий Qt Widget)
│   └── Right: DashboardWidget (QQuickWidget wrapper)
│       └── Dashboard.qml
│           ├── 3D Scene (Qt Quick 3D)
│           │   ├── Camera (изометрия, угол 45°)
│           │   ├── CarModel (прямоугольник в центре)
│           │   ├── RoadPlane (20x40 метров)
│           │   └── Repeater3D → MarkingLine objects
│           │
│           └── 2D Overlays (поверх 3D)
│               ├── WarningPanel (слева сверху)
│               ├── CenterOffsetIndicator (слева снизу)
│               └── ConnectionStatus (справа снизу)
│
└── [Bottom] Status Bar
```

---

## Ключевые компоненты

### 1. DashboardWidget (C++)

**Расположение**: `ui/DashboardWidget.hpp/cpp`

**Назначение**: C++ обертка вокруг QQuickWidget для интеграции QML в Qt Widgets приложение

**Обязанности**:
- Создание QQuickWidget
- Загрузка `Dashboard.qml`
- Регистрация ViewModels в QML context через `setContextProperty()`
- Настройка темной темы для QML
- Обработка ошибок загрузки QML

**API**:
```cpp
class DashboardWidget : public QQuickWidget {
    Q_OBJECT
public:
    explicit DashboardWidget(QWidget* parent = nullptr);
    void setAppController(app::AppController* controller);
private:
    app::AppController* controller_{nullptr};
};
```

**Регистрируемые context properties**:
- `markingModel` → MarkingObjectListModel*
- `laneViewModel` → LaneStateViewModel*
- `warningModel` → WarningListModel*
- `appController` → AppController* (для isDataConnected, isVideoConnected)

---

### 2. Dashboard.qml (Root)

**Расположение**: `qml/Dashboard.qml`

**Структура**:
- Темный фон (#1a1a1a)
- View3D с 3D сценой
- 2D overlays поверх 3D сцены

**Назначение**: Корневой QML компонент, содержит всю dashboard панель

---

### 3. CarScene.qml (Qt Quick 3D Scene)

**Расположение**: `qml/CarScene.qml`

**Элементы сцены**:

#### Камера
- Тип: PerspectiveCamera
- Позиция: сзади сверху от автомобиля
- Угол: изометрия (~45° вниз)
- FOV: 60°

#### Освещение
- DirectionalLight для общего освещения
- Возможно ambient light для теней

#### Автомобиль (CarModel)
- Позиция: центр сцены (0, 0, 0)
- Простая геометрия: Cube с масштабом (1.8 x 0.5 x 4.5 метров)
- Материал: синий, с металлическим блеском

#### Дорожное полотно (опционально)
- RoadPlane: 20м ширина x 40м длина
- Серый асфальт с текстурой или простой цвет

#### Объекты разметки
- Repeater3D с model: markingModel
- Delegate: MarkingLine component
- Динамически создаются/удаляются при изменении модели

**Координатная система**:
- **X**: поперек дороги (-10м ... +10м, left-right)
- **Y**: высота (обычно 0 для дороги, 0.01 для разметки)
- **Z**: вдоль дороги (0 = автомобиль, +40м впереди, -10м позади)

**Масштаб**: фиксированная область 20x40 метров

---

### 4. MarkingLine.qml

**Расположение**: `qml/MarkingLine.qml`

**Назначение**: 3D представление одного объекта дорожной разметки

**Данные из MarkingObjectListModel**:
- `centerX`, `centerY` - координаты в метрах от автомобиля
- `width`, `length` - размеры объекта
- `type` - тип разметки (для выбора цвета)

**Рендеринг**:
- Model с source: "#Cube"
- Position из centerX/centerY
- Scale из width/length
- Цвет зависит от типа разметки (SOLID_WHITE → белый, DASHED_YELLOW → желтый, и т.д.)
- emissiveColor для свечения и видимости

**Маппинг типов на цвета**:
- SOLID_WHITE → #ffffff
- DASHED_YELLOW → #ffff00
- SOLID_YELLOW → #ffcc00
- DASHED_WHITE → #cccccc
- и т.д.

---

### 5. WarningPanel.qml

**Расположение**: `qml/WarningPanel.qml`

**Позиция**: слева сверху, margins 20px

**Отображаемые варнинги** (из WarningListModel):
- Lane departure (выезд из полосы) - ⚠
- Poor lane quality - ⚡
- No lane detected - ✖

**Визуализация**:
- Column с Repeater
- Каждый варнинг = Rectangle с закругленными углами
- Цвет по severity:
  - CRITICAL (0) → красный (#e74c3c)
  - WARNING (1) → оранжевый (#f39c12)
  - INFO (2) → синий (#3498db)
- Row: символ + текст сообщения
- Автоматически появляются/исчезают при изменении модели

---

### 6. CenterOffsetIndicator.qml

**Расположение**: `qml/CenterOffsetIndicator.qml`

**Позиция**: слева снизу, margins 20px

**Назначение**: Визуальный индикатор смещения автомобиля от центра полосы

**Элементы**:
- Заголовок "Center Offset"
- Горизонтальный progress bar стиль индикатор
- Центральная линия (идеальная позиция)
- Движущийся маркер (текущая позиция)
- Числовое значение в метрах

**Данные**:
- Binding к `laneViewModel.centerOffsetMeters`
- Диапазон: -1.0м до +1.0м

**Цветовая индикация маркера**:
- < 0.3м от центра → зеленый (хорошо)
- 0.3-0.6м → оранжевый (предупреждение)
- > 0.6м → красный (опасно)

---

### 7. ConnectionStatus.qml

**Расположение**: `qml/ConnectionStatus.qml`

**Позиция**: справа снизу, margins 20px

**Отображает**:
- DATA connection status (appController.isDataConnected)
- VIDEO connection status (appController.isVideoConnected)

**Визуализация**:
- Row из двух StatusIndicator компонентов
- Каждый: круглый индикатор + label
- Цвет: зеленый (connected) / красный (disconnected)

---

## Поток данных

```
Network Layer
    ↓
ConnectionManager
    ├── onFrameReceived() → updates domain models
    ├── emits: laneStateUpdated()
    ├── emits: markingModelUpdated()
    └── emits: warningModelUpdated()
        ↓
AppController (slots обрабатывают сигналы)
    ├── onLaneStateUpdated() → updates sync monitor
    ├── onMarkingModelUpdated() → triggers ViewModel update
    └── onWarningModelUpdated()
        ↓
ViewModels (автоматически обновляются)
    ├── LaneStateViewModel (Q_PROPERTY + signals)
    ├── MarkingObjectListModel (QAbstractListModel, roles)
    └── WarningListModel (QAbstractListModel, roles)
        ↓
QML Context Properties (зарегистрированы в DashboardWidget)
    ├── laneViewModel
    ├── markingModel
    ├── warningModel
    └── appController
        ↓
QML Property Bindings (автоматически реагируют на изменения)
    ├── CenterOffsetIndicator { offset: laneViewModel.centerOffsetMeters }
    ├── Repeater3D { model: markingModel }
    └── WarningPanel Repeater { model: warningModel }
        ↓
UI Updates (Qt Quick автоматически перерисовывает)
```

**Важно**: Вся цепочка реактивная благодаря Qt signals/slots и QML property bindings. Никакой ручной синхронизации не требуется!

---

## Модификации существующих компонентов

### MainWindow.cpp

**Изменения**:

1. **Удалить старые info panels**:
   - `setupInfoPanels()` - удалить
   - `lane_info_panel_`, `sync_info_panel_` - удалить
   - Слоты `onLaneStateChanged()`, `onSyncStatusChanged()` - удалить
   - Эта информация теперь в QML dashboard

2. **Заменить setupVideoPanel() на setupContentPanel()**:
   - Создать QSplitter (horizontal)
   - Добавить NetworkVideoWidget слева
   - Создать DashboardWidget справа
   - Вызвать `dashboardWidget->setAppController(controller_)`
   - Установить stretch factors 1:1 (50/50)

3. **Сохранить**:
   - Control panel сверху (host/port/connect)
   - Status bar снизу (может быть упрощен, т.к. connection status теперь в QML)

### MainWindow.hpp

**Добавить**:
```cpp
#include "DashboardWidget.hpp"

private:
    QSplitter* content_splitter_{nullptr};
    ui::DashboardWidget* dashboard_widget_{nullptr};
```

**Удалить**:
- `lane_info_panel_`, `sync_info_panel_`, `warning_panel_`
- Связанные labels и слоты

---

## Структура файлов

```
Dashboard-for-road-marking/
├── ui/
│   ├── MainWindow.hpp/cpp          (модифицировать)
│   └── DashboardWidget.hpp/cpp     (создать новый)
│
├── qml/
│   ├── Dashboard.qml               (root QML)
│   ├── CarScene.qml                (3D сцена с камерой)
│   ├── CarModel.qml                (3D модель автомобиля)
│   ├── MarkingLine.qml             (3D линия разметки)
│   ├── RoadPlane.qml               (опционально - дорога)
│   ├── WarningPanel.qml            (overlay - варнинги)
│   ├── CenterOffsetIndicator.qml   (overlay - offset)
│   └── ConnectionStatus.qml        (overlay - connection)
│
└── CMakeLists.txt                  (модифицировать)
```

---

## CMakeLists.txt изменения

**Добавить Qt компоненты**:
```cmake
find_package(Qt6 REQUIRED COMPONENTS
    Qml
    Quick
    Quick3D
)
```

**Зарегистрировать QML модуль**:
```cmake
qt_add_qml_module(dashboard
    URI DashboardQml
    VERSION 1.0
    QML_FILES
        qml/Dashboard.qml
        qml/CarScene.qml
        qml/CarModel.qml
        qml/MarkingLine.qml
        qml/RoadPlane.qml
        qml/WarningPanel.qml
        qml/CenterOffsetIndicator.qml
        qml/ConnectionStatus.qml
)
```

**Добавить линковку**:
```cmake
target_link_libraries(dashboard PRIVATE
    Qt6::Qml
    Qt6::Quick
    Qt6::Quick3D
)
```

---

## Важные нюансы реализации

### 1. MarkingObjectListModel - QML роли

Убедиться что MarkingObjectListModel экспортирует роли для QML:

```cpp
QHash<int, QByteArray> MarkingObjectListModel::roleNames() const {
    return {
        {CenterXRole, "centerX"},
        {CenterYRole, "centerY"},
        {WidthRole, "width"},
        {LengthRole, "length"},
        {TypeRole, "type"},
        // ... другие роли
    };
}
```

В QML доступ: `model.centerX`, `model.type`, и т.д.

### 2. Координаты MarkingObject → 3D сцена

MarkingObject уже содержит координаты в метрах относительно автомобиля.

**Прямой маппинг**:
- MarkingObject.centerX → position.x в QML
- MarkingObject.centerY → position.z в QML
- Y координата (высота) всегда 0.01 (чуть выше дороги)

**Не требуется дополнительных трансформаций**, если:
- Автомобиль в центре (0, 0, 0)
- X - поперек дороги
- Z - вдоль дороги

### 3. QQuickWidget vs QQuickView

Используем **QQuickWidget** потому что:
- Легко интегрируется в QWidget-based UI
- Можно добавить в QSplitter
- Проще настройка context properties

Минус: чуть медленнее рендеринг чем QQuickView. Но для нашей сцены (небольшое количество объектов) производительности достаточно.

### 4. Темная тема для QML

В DashboardWidget конструкторе:
```cpp
setResizeMode(QQuickWidget::SizeRootObjectToView);
setClearColor(QColor("#1a1a1a"));  // Темный фон
```

### 5. Обработка ошибок загрузки QML

```cpp
connect(engine(), &QQmlEngine::warnings, [](const QList<QQmlError>& warnings) {
    for (const auto& warning : warnings) {
        LOG_WARN << "QML Warning: " << warning.toString().toStdString();
    }
});
```

### 6. Qt Quick 3D материалы

Для лучшей видимости на темном фоне:
- Использовать `emissiveColor` для разметки (свечение)
- Ambient light чтобы все объекты были видны
- Metalness/Roughness для автомобиля

### 7. WarningListModel триггеры

Убедиться что WarningListModel правильно эмитит:
- `rowsInserted()` когда добавляется варнинг
- `rowsRemoved()` когда варнинг исчезает
- `dataChanged()` когда изменяется существующий варнинг

Иначе QML Repeater не обновится.

### 8. Performance considerations

**Qt Quick 3D оптимизации**:
- Repeater3D создает объекты только для видимых элементов модели
- Если markingModel содержит > 100 объектов, рассмотреть instancing
- Простая геометрия (#Cube) рендерится очень быстро

**Если тормозит**:
- Уменьшить количество объектов разметки (фильтровать по расстоянию)
- Использовать LOD (Level of Detail) для дальних объектов
- Снизить качество теней/освещения

### 9. Тестирование без реальных данных

Для тестирования QML UI без подключения к серверу:

**Mock данные в DashboardWidget**:
```cpp
// Временно для теста
auto* mockMarking = new viewmodels::MarkingObjectListModel(this);
// Добавить тестовые объекты
rootContext()->setContextProperty("markingModel", mockMarking);
```

Или использовать существующие пустые модели - UI должен корректно отображаться даже если модели пустые.

### 10. QML debugging

Включить QML debugger в CMakeLists.txt (только для разработки):
```cmake
target_compile_definitions(dashboard PRIVATE QT_QML_DEBUG)
```

Использовать `qmlscene` для быстрого тестирования отдельных QML файлов.

---

## План разработки (TODO)

### Фаза 1: Инфраструктура (Setup)
- [ ] Обновить CMakeLists.txt - добавить Qt Quick/Quick3D компоненты
- [ ] Создать директорию `qml/`
- [ ] Создать заглушки всех QML файлов (пустые Item {})
- [ ] Создать DashboardWidget.hpp/cpp (базовая структура)
- [ ] Протестировать компиляцию

### Фаза 2: DashboardWidget (C++ интеграция)
- [ ] Реализовать DashboardWidget конструктор
- [ ] Загрузка Dashboard.qml из ресурсов
- [ ] Реализовать setAppController() - регистрация context properties
- [ ] Добавить обработку ошибок QML
- [ ] Настроить темную тему и размеры
- [ ] Протестировать загрузку пустого Dashboard.qml

### Фаза 3: MainWindow модификации
- [ ] Удалить старые info panels из MainWindow.hpp/cpp
- [ ] Удалить слоты onLaneStateChanged(), onSyncStatusChanged()
- [ ] Заменить setupVideoPanel() на setupContentPanel()
- [ ] Создать QSplitter и добавить NetworkVideoWidget + DashboardWidget
- [ ] Обновить setupConnections() - убрать ненужные
- [ ] Протестировать: окно делится 50/50, видео слева, черный QML справа

### Фаза 4: 3D Scene (базовая сцена)
- [ ] Реализовать CarScene.qml - PerspectiveCamera, DirectionalLight
- [ ] Настроить камеру (позиция, угол, FOV)
- [ ] Реализовать CarModel.qml - простой синий Cube
- [ ] Добавить CarScene в Dashboard.qml
- [ ] Протестировать: видна 3D сцена с автомобилем

### Фаза 5: Дорожная разметка (динамические объекты)
- [ ] Проверить MarkingObjectListModel::roleNames() - экспорт ролей для QML
- [ ] Реализовать MarkingLine.qml - 3D представление одного объекта
- [ ] Добавить Repeater3D в CarScene.qml с model: markingModel
- [ ] Реализовать маппинг типов разметки на цвета
- [ ] Протестировать с mock данными или реальным подключением

### Фаза 6: Road Plane (опционально)
- [ ] Реализовать RoadPlane.qml - серая плоскость 20x40м
- [ ] Добавить в CarScene
- [ ] Опционально: текстура асфальта

### Фаза 7: Warning Panel (overlay)
- [ ] Реализовать WarningPanel.qml
- [ ] Repeater с model: warningModel
- [ ] Цветные блоки с символами и текстом
- [ ] Маппинг severity на цвета
- [ ] Позиционирование слева сверху
- [ ] Протестировать с различными варнингами

### Фаза 8: Center Offset Indicator
- [ ] Реализовать CenterOffsetIndicator.qml
- [ ] Progress bar стиль индикатор с центральной линией
- [ ] Binding к laneViewModel.centerOffsetMeters
- [ ] Движущийся маркер с цветовой индикацией
- [ ] Числовое значение
- [ ] Позиционирование слева снизу
- [ ] Протестировать с различными offset значениями

### Фаза 9: Connection Status
- [ ] Реализовать ConnectionStatus.qml
- [ ] Создать переиспользуемый компонент StatusIndicator
- [ ] Два индикатора: DATA и VIDEO
- [ ] Binding к appController.isDataConnected, isVideoConnected
- [ ] Позиционирование справа снизу
- [ ] Протестировать подключение/отключение

### Фаза 10: Интеграция и тестирование
- [ ] Собрать все компоненты в Dashboard.qml
- [ ] Проверить layout всех overlays (не перекрываются ли)
- [ ] Тестирование с реальным подключением к серверу
- [ ] Проверить что все ViewModels обновляют QML
- [ ] Проверить что разметка появляется в правильных позициях
- [ ] Проверить варнинги появляются/исчезают корректно

### Фаза 11: Полировка
- [ ] Настроить освещение 3D сцены для лучшей видимости
- [ ] Добавить transitions/animations для warning panel (плавное появление)
- [ ] Настроить масштаб 3D сцены (чтобы все объекты влезали)
- [ ] Проверить производительность (FPS)
- [ ] Настроить цвета и стили для consistency
- [ ] Опционально: добавить grid на дороге для масштаба

### Фаза 12: Документация и cleanup
- [ ] Обновить комментарии в коде
- [ ] Создать screenshots нового UI
- [ ] Обновить README с новой архитектурой
- [ ] Удалить неиспользуемый код из старых info panels
- [ ] Code review всех изменений

---

## Проверочный список (Checklist)

После завершения всех фаз убедиться что:

### Функциональность
- ✅ Видео отображается слева корректно
- ✅ 3D сцена справа с автомобилем в центре
- ✅ Объекты разметки появляются в правильных позициях относительно автомобиля
- ✅ Варнинги появляются/исчезают в реальном времени
- ✅ Center offset индикатор обновляется при движении
- ✅ Connection status индикаторы работают
- ✅ QSplitter позволяет менять размеры панелей
- ✅ Control panel сверху работает (connect/disconnect)

### Производительность
- ✅ UI отзывчивый, нет лагов
- ✅ 3D сцена рендерится плавно (>30 FPS)
- ✅ Обновления ViewModels не тормозят UI
- ✅ Нет утечек памяти при долгой работе

### Визуальное качество
- ✅ Темная тема согласована во всех компонентах
- ✅ Текст читаемый на всех фонах
- ✅ Цвета варнингов интуитивные (красный=опасность)
- ✅ 3D объекты хорошо видны (достаточное освещение)
- ✅ Overlays не загораживают важную информацию

### Надежность
- ✅ Нет ошибок в QML (проверить Qt Creator Application Output)
- ✅ Корректная обработка пустых моделей (нет крашей)
- ✅ Корректная обработка отсутствия данных
- ✅ Graceful degradation при отключении от сервера

---

## Дополнительные улучшения (опционально)

После основной реализации можно рассмотреть:

1. **Анимации**:
   - Плавное появление/исчезновение варнингов (Behavior on opacity)
   - Мигание критических варнингов
   - Плавное движение offset индикатора

2. **Дополнительная информация**:
   - Lane width индикатор
   - FPS counter для видео
   - Timestamp синхронизации

3. **Интерактивность**:
   - Клик на варнинг → подсветка связанного объекта разметки
   - Zoom in/out для 3D сцены (mouse wheel)
   - Вращение камеры (опционально)

4. **Улучшенная 3D визуализация**:
   - Более реалистичная модель автомобиля (загрузка .obj/.gltf)
   - Текстура дороги
   - Тени от объектов
   - Fog для дальних объектов

5. **Конфигурация**:
   - Настройка размера видимой области (10x20м или 20x40м)
   - Настройка цветов разметки
   - Включение/выключение отдельных overlays

---

## Возможные проблемы и решения

### Проблема: QML не видит context properties

**Причина**: setContextProperty() вызван после setSource()

**Решение**: Всегда регистрировать properties ДО загрузки QML:
```cpp
rootContext()->setContextProperty("model", model);
setSource(QUrl("qrc:/qml/Dashboard.qml"));
```

### Проблема: Repeater3D не обновляется

**Причина**: MarkingObjectListModel не эмитит правильные сигналы

**Решение**: Проверить что при изменении вызываются:
- beginInsertRows() / endInsertRows()
- beginRemoveRows() / endRemoveRows()
- dataChanged()

### Проблема: 3D объекты не видны

**Причины**:
- Неправильная позиция камеры
- Отсутствие освещения
- Объекты слишком маленькие
- Объекты за пределами frustum

**Решение**:
- Добавить ambient light
- Проверить координаты объектов (qDebug)
- Увеличить emissiveColor
- Настроить camera near/far planes

### Проблема: Тормозит при большом количестве объектов

**Решение**:
- Фильтровать объекты по расстоянию (показывать только ближайшие 50)
- Использовать instancing для одинаковых объектов
- Упростить геометрию
- Снизить качество материалов

### Проблема: Memory leak

**Причина**: Context properties содержат сырые указатели без владения

**Решение**: AppController владеет всеми ViewModels, они будут удалены при его удалении. QQuickWidget не владеет context properties - это нормально.

---

## Заключение

Данная архитектура позволяет:
- Использовать существующую Qt Widgets инфраструктуру
- Добавить современный QML UI для приборной панели
- Повторно использовать все существующие ViewModels без изменений
- Легко расширять функциональность в будущем

Следуя плану разработки по фазам, можно постепенно внедрить новый UI с минимальным риском поломки существующего функционала.
