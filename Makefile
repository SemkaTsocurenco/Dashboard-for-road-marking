# Makefile для проекта Dashboard
# Быстрые команды для сборки и управления проектом

.PHONY: all build clean rebuild run configure help install

# Директории
BUILD_DIR = build
SRC_DIR = .

# Количество потоков для сборки
JOBS = $(shell nproc)

# По умолчанию - сборка проекта
all: build

# Конфигурация проекта через CMake
configure:
	@echo "=== Конфигурация проекта с CMake ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles"
	@echo "✓ Конфигурация завершена"

# Сборка проекта
build: configure
	@echo "=== Сборка проекта ==="
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)
	@echo "✓ Сборка завершена"
	@echo "Исполняемый файл: $(BUILD_DIR)/dashboard"

# Сборка в Release режиме
release:
	@echo "=== Сборка проекта (Release) ==="
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles"
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)
	@echo "✓ Release сборка завершена"

# Быстрая пересборка (без пересоздания CMake)
fast:
	@echo "=== Быстрая сборка ==="
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)
	@echo "✓ Быстрая сборка завершена"

# Очистка build директории
clean:
	@echo "=== Очистка проекта ==="
	@if [ -d $(BUILD_DIR) ]; then \
		$(MAKE) -C $(BUILD_DIR) clean; \
		echo "✓ Очистка завершена"; \
	else \
		echo "Директория build не существует"; \
	fi

# Полная очистка (удаление build директории)
distclean:
	@echo "=== Полная очистка проекта ==="
	@rm -rf $(BUILD_DIR)
	@echo "✓ Директория build удалена"

# Пересборка с нуля
rebuild: distclean build

# Запуск приложения
run: build
	@echo "=== Запуск приложения ==="
	@./$(BUILD_DIR)/dashboard

# Установка зависимостей (Qt6, OpenCV)
install-deps:
	@echo "=== Установка зависимостей ==="
	@echo "Установка Qt6..."
	@sudo apt-get update
	@sudo apt-get install -y qt6-base-dev libqt6core6 libqt6gui6 libqt6widgets6
	@echo "Установка OpenCV (опционально)..."
	@sudo apt-get install -y libopencv-dev
	@echo "Установка CMake и компиляторов..."
	@sudo apt-get install -y cmake build-essential
	@echo "✓ Зависимости установлены"

# Установка Clang (опционально)
install-clang:
	@echo "=== Установка Clang ==="
	@sudo apt-get update
	@sudo apt-get install -y clang
	@echo "✓ Clang установлен"

# Проверка стиля кода (если установлен clang-format)
format:
	@echo "=== Форматирование кода ==="
	@if command -v clang-format >/dev/null 2>&1; then \
		find $(SRC_DIR) -name "*.cpp" -o -name "*.h" | xargs clang-format -i; \
		echo "✓ Форматирование завершено"; \
	else \
		echo "clang-format не установлен. Установите: sudo apt-get install clang-format"; \
	fi

# Статический анализ (если установлен clang-tidy)
check:
	@echo "=== Статический анализ кода ==="
	@if command -v clang-tidy >/dev/null 2>&1; then \
		clang-tidy $(SRC_DIR)/*.cpp -- -I/usr/include/qt6; \
		echo "✓ Анализ завершен"; \
	else \
		echo "clang-tidy не установлен. Установите: sudo apt-get install clang-tidy"; \
	fi

# Информация о проекте
info:
	@echo "=== Информация о проекте ==="
	@echo "Название: Dashboard"
	@echo "Build директория: $(BUILD_DIR)"
	@echo "Количество потоков: $(JOBS)"
	@echo "Компилятор: $$(which g++ || which clang++)"
	@if [ -f $(BUILD_DIR)/dashboard ]; then \
		echo "Исполняемый файл: $(BUILD_DIR)/dashboard"; \
		ls -lh $(BUILD_DIR)/dashboard; \
	else \
		echo "Исполняемый файл не собран"; \
	fi

# Справка по командам
help:
	@echo "=== Доступные команды Makefile ==="
	@echo ""
	@echo "Основные команды:"
	@echo "  make              - Сборка проекта (configure + build)"
	@echo "  make build        - Полная сборка проекта"
	@echo "  make fast         - Быстрая пересборка (без CMake)"
	@echo "  make run          - Сборка и запуск приложения"
	@echo "  make clean        - Очистка скомпилированных файлов"
	@echo "  make distclean    - Полная очистка (удаление build/)"
	@echo "  make rebuild      - Пересборка с нуля (distclean + build)"
	@echo ""
	@echo "Конфигурация:"
	@echo "  make configure    - Конфигурация CMake"
	@echo "  make release      - Сборка в Release режиме"
	@echo ""
	@echo "Установка:"
	@echo "  make install-deps - Установка зависимостей (Qt6, OpenCV)"
	@echo "  make install-clang- Установка компилятора Clang"
	@echo ""
	@echo "Качество кода:"
	@echo "  make format       - Форматирование кода (clang-format)"
	@echo "  make check        - Статический анализ (clang-tidy)"
	@echo ""
	@echo "Информация:"
	@echo "  make info         - Информация о проекте"
	@echo "  make help         - Эта справка"
	@echo ""
