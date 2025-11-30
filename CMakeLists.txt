cmake_minimum_required(VERSION 3.16)

# Попытка использовать Clang, если доступен
find_program(CLANG_CXX_COMPILER clang++)
if(CLANG_CXX_COMPILER)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
    message(STATUS "Using Clang compiler: ${CLANG_CXX_COMPILER}")
else()
    message(STATUS "Clang not found, using default compiler")
endif()

project(dashboard LANGUAGES CXX)

# Стандарт C++ и настройки Qt
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Экспорт compile_commands.json для автодополнения
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Поиск зависимостей
find_package(PkgConfig REQUIRED)
find_package(Qt6 COMPONENTS Core Gui Widgets Network Multimedia Qml Quick Quick3D QuickWidgets REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)

# OpenCV по умолчанию отключён, т.к. системная сборка тянет Qt5 и конфликтует с Qt6.
# Включайте только если у вас OpenCV, собранный с Qt6.
option(ENABLE_OPENCV "Enable OpenCV support (requires OpenCV built with Qt6)" OFF)
set(HAVE_OPENCV FALSE)
if(ENABLE_OPENCV)
    find_package(OpenCV QUIET)
    if(OpenCV_FOUND)
        message(STATUS "OpenCV found: ${OpenCV_VERSION}")
        set(HAVE_OPENCV TRUE)
    else()
        message(WARNING "OpenCV requested but not found - building without OpenCV support")
    endif()
endif()

include_directories(
    app/
    config/
    ui/
    logger/
    network/
    parser/
    domain/
    viewmodels/
    videowidget/
    videowidget/interfaces/
    videowidget/base/
    videowidget/src/
    videowidget/widgets/
    videowidget/processors/
)

# Сбор всех исходных файлов (исключая build директорию)
file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cpp
)

file(GLOB_RECURSE HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.hpp
)

# Исключаем файлы из build директории
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/build/.*")
list(FILTER HEADERS EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/build/.*")

# Создание исполняемого файла
add_executable(dashboard
    ${SOURCES}
)

# Подключение заголовочных файлов
target_include_directories(dashboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GSTREAMER_INCLUDE_DIRS}
)

# Добавление OpenCV, если найден
if(HAVE_OPENCV)
    target_include_directories(dashboard PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_compile_definitions(dashboard PRIVATE HAVE_OPENCV)
endif()


# Линковка библиотек
target_link_libraries(dashboard
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::Qml
        Qt6::Quick
        Qt6::Quick3D
        Qt6::QuickWidgets
        ${GSTREAMER_LIBRARIES}
)

if(HAVE_OPENCV)
    target_link_libraries(dashboard PRIVATE ${OpenCV_LIBS})
endif()

target_compile_options(dashboard PRIVATE ${GSTREAMER_CFLAGS_OTHER})
target_link_directories(dashboard PRIVATE ${GSTREAMER_LIBRARY_DIRS})


# Подключаем QML-ресурсы (AUTORCC соберёт .qrc)
target_sources(dashboard PRIVATE
    resources/resources.qrc
)
