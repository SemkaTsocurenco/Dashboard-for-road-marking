# Установка и сборка

## Системные требования

### Минимальные требования

| Компонент | Требование |
|-----------|------------|
| ОС | Linux (Ubuntu 20.04+), Windows 10+ |
| Процессор | x86_64, 2+ ядра |
| RAM | 4 GB |
| GPU | OpenGL 3.3+ (для Qt Quick 3D) |
| Дисковое пространство | 500 MB |

### Рекомендуемые требования

| Компонент | Требование |
|-----------|------------|
| ОС | Ubuntu 22.04 LTS |
| Процессор | x86_64, 4+ ядра |
| RAM | 8 GB |
| GPU | Дискретная видеокарта с OpenGL 4.5+ |

## Зависимости

### Обязательные зависимости

| Зависимость | Версия | Назначение |
|-------------|--------|------------|
| CMake | 3.16+ | Система сборки |
| Qt | 6.2+ | UI фреймворк |
| GStreamer | 1.0+ | Видеопотоки |
| GCC/Clang | C++17 | Компилятор |

### Qt модули

```
Qt6::Core
Qt6::Gui
Qt6::Widgets
Qt6::Network
Qt6::Multimedia
Qt6::Qml
Qt6::Quick
Qt6::Quick3D
Qt6::QuickWidgets
```

### Опциональные зависимости

| Зависимость | Назначение |
|-------------|------------|
| OpenCV | Дополнительная обработка изображений |
| Clang | Альтернативный компилятор |

## Установка зависимостей

### Ubuntu/Debian

```bash
# Обновление пакетов
sudo apt update && sudo apt upgrade -y

# Установка инструментов сборки
sudo apt install -y build-essential cmake git

# Установка Qt6
sudo apt install -y \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-multimedia-dev \
    qt6-quick3d-dev \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick3d \
    qml6-module-qtquick-layouts \
    libqt6opengl6-dev

# Установка GStreamer
sudo apt install -y \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-tools

# Опционально: OpenCV
sudo apt install -y libopencv-dev
```

### Fedora

```bash
# Установка инструментов сборки
sudo dnf install -y cmake gcc-c++ git

# Установка Qt6
sudo dnf install -y \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qtmultimedia-devel \
    qt6-qt3d-devel \
    qt6-qtquick3d-devel

# Установка GStreamer
sudo dnf install -y \
    gstreamer1-devel \
    gstreamer1-plugins-base-devel \
    gstreamer1-plugins-good \
    gstreamer1-plugins-bad-free \
    gstreamer1-plugins-ugly-free
```

### Arch Linux

```bash
# Установка зависимостей
sudo pacman -S --needed \
    base-devel cmake git \
    qt6-base qt6-declarative qt6-multimedia qt6-quick3d \
    gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly
```

### Windows

1. **Установите Qt:**
   - Скачайте Qt Online Installer: https://www.qt.io/download
   - Установите Qt 6.2+ с компонентами:
     - Qt Quick 3D
     - Qt Multimedia
     - MinGW или MSVC compiler

2. **Установите CMake:**
   - Скачайте с https://cmake.org/download/
   - Добавьте в PATH

3. **Установите GStreamer:**
   - Скачайте с https://gstreamer.freedesktop.org/download/
   - Установите runtime и development пакеты

## Сборка проекта

### 1. Клонирование репозитория

```bash
git clone <repository-url>
cd Dashboard-for-road-marking
```

### 2. Создание директории сборки

```bash
mkdir build
cd build
```

### 3. Конфигурация CMake

**Стандартная сборка:**

```bash
cmake ..
```

**С указанием компилятора:**

```bash
cmake -DCMAKE_CXX_COMPILER=clang++ ..
```

**Release сборка:**

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

**Debug сборка:**

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

**С OpenCV:**

```bash
cmake -DUSE_OPENCV=ON ..
```

### 4. Компиляция

```bash
# Использовать все ядра процессора
make -j$(nproc)

# Или с указанием количества потоков
make -j4
```

### 5. Запуск

```bash
./Dashboard-for-road-marking
```

## Опции CMake

| Опция | По умолчанию | Описание |
|-------|--------------|----------|
| `CMAKE_BUILD_TYPE` | `Debug` | Тип сборки (Debug/Release) |
| `USE_OPENCV` | `OFF` | Включить поддержку OpenCV |
| `CMAKE_CXX_COMPILER` | system | Компилятор C++ |

## Проверка установки

### Проверка Qt

```bash
qmake6 --version
# или
qmake --version
```

### Проверка GStreamer

```bash
gst-inspect-1.0 --version
gst-launch-1.0 videotestsrc ! autovideosink
```

### Проверка OpenGL

```bash
glxinfo | grep "OpenGL version"
```

## Устранение проблем

### Ошибка: Qt6 не найден

```
Could not find a package configuration file provided by "Qt6"
```

**Решение:**

```bash
# Укажите путь к Qt
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 ..
```

### Ошибка: GStreamer не найден

```
Could not find GStreamer
```

**Решение:**

```bash
# Ubuntu
sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

# Укажите путь вручную
cmake -DGSTREAMER_INCLUDE_DIRS=/usr/include/gstreamer-1.0 ..
```

### Ошибка: Qt Quick 3D не работает

```
QML module "QtQuick3D" is not installed
```

**Решение:**

```bash
# Ubuntu
sudo apt install qml6-module-qtquick3d qt6-quick3d-dev

# Проверка
qml6 -e "import QtQuick3D; print('OK')"
```

### Чёрный экран в 3D сцене

**Причина:** Нет поддержки OpenGL или драйверы не установлены.

**Решение:**

```bash
# Установка драйверов NVIDIA
sudo apt install nvidia-driver-XXX

# Или Mesa для встроенной графики
sudo apt install mesa-utils

# Проверка
glxinfo | grep "direct rendering"
# Должно быть: direct rendering: Yes
```

### Ошибка сегментации при запуске

**Возможные причины:**
1. Несовместимая версия Qt
2. Отсутствуют QML плагины

**Решение:**

```bash
# Проверка QML импортов
QML_IMPORT_TRACE=1 ./Dashboard-for-road-marking

# Установка недостающих модулей
sudo apt install qml6-module-qtquick-controls qml6-module-qtquick-layouts
```

## Структура директории сборки

После успешной сборки:

```
build/
├── Dashboard-for-road-marking    # Исполняемый файл
├── CMakeCache.txt                # Кэш CMake
├── CMakeFiles/                   # Файлы CMake
├── Makefile                      # Makefile
└── compile_commands.json         # Для IDE (если включено)
```

## Развёртывание

### Linux

```bash
# Копирование бинарника
cp build/Dashboard-for-road-marking /usr/local/bin/

# Копирование конфигурации
cp config.json /etc/dashboard-marking/
cp scene_config.json /etc/dashboard-marking/
```

### Создание .deb пакета

```bash
# Установка инструментов
sudo apt install checkinstall

# Создание пакета
cd build
sudo checkinstall --pkgname=dashboard-marking --pkgversion=1.0
```

### AppImage (Linux)

```bash
# Установка linuxdeployqt
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt-continuous-x86_64.AppImage

# Создание AppImage
./linuxdeployqt-continuous-x86_64.AppImage Dashboard-for-road-marking -appimage
```
