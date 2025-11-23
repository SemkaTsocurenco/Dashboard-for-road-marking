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
find_package(Qt6 COMPONENTS Core Gui Widgets Network Multimedia REQUIRED)

# OpenCV опционально (для обработки изображений)
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    set(HAVE_OPENCV TRUE)
else()
    message(STATUS "OpenCV not found - building without OpenCV support")
    set(HAVE_OPENCV FALSE)
endif()

include_directories(
    logger/
    network/
    parser/
    domain/
    viewmodels/
    videowidget/
    videowidget/interfaces/
    videowidget/base/
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
)

# Линковка библиотек
target_link_libraries(dashboard
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
)

# Добавление OpenCV, если найден
if(HAVE_OPENCV)
    target_include_directories(dashboard PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(dashboard ${OpenCV_LIBS})
    target_compile_definitions(dashboard PRIVATE HAVE_OPENCV)
endif()