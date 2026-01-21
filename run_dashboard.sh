#!/bin/bash
# Скрипт для запуска Dashboard в чистом окружении без snap-библиотек

# Сохраняем важные переменные окружения
DISPLAY_VAR=${DISPLAY:-:0}
HOME_VAR=${HOME}
USER_VAR=${USER}
WAYLAND_DISPLAY_VAR=${WAYLAND_DISPLAY}
XDG_RUNTIME_DIR_VAR=${XDG_RUNTIME_DIR}

# Запускаем в минимальном окружении с правильными путями к библиотекам
env -i \
  HOME="$HOME_VAR" \
  USER="$USER_VAR" \
  DISPLAY="$DISPLAY_VAR" \
  WAYLAND_DISPLAY="$WAYLAND_DISPLAY_VAR" \
  XDG_RUNTIME_DIR="$XDG_RUNTIME_DIR_VAR" \
  PATH="/usr/local/bin:/usr/bin:/bin" \
  LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu" \
  QT_QPA_PLATFORM_PLUGIN_PATH="/usr/lib/x86_64-linux-gnu/qt6/plugins/platforms" \
  QT_PLUGIN_PATH="/usr/lib/x86_64-linux-gnu/qt6/plugins" \
  ./build/dashboard "$@"
