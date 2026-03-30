#!/bin/bash

# Raspberry Pi connection settings
RPI_HOST="192.168.31.179"
RPI_USER="semka"
RPI_PROJECT_DIR="/home/semka/dashboard"

# Local project directory
LOCAL_PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Dashboard Deploy Script ===${NC}"
echo "Target: ${RPI_USER}@${RPI_HOST}:${RPI_PROJECT_DIR}"

# Function to run SSH commands
ssh_cmd() {
    ssh "${RPI_USER}@${RPI_HOST}" "$@"
}

# Function to check if dependencies are installed on RPi
check_dependencies() {
    echo -e "${YELLOW}Checking dependencies on Raspberry Pi...${NC}"

    ssh_cmd "which cmake > /dev/null 2>&1" || {
        echo -e "${RED}CMake not found. Installing...${NC}"
        ssh_cmd "sudo apt update && sudo apt install -y cmake build-essential"
    }

    ssh_cmd "pkg-config --exists Qt6Core 2>/dev/null" || {
        echo -e "${YELLOW}Qt6 not found. Installing required packages...${NC}"
        ssh_cmd "sudo apt update && sudo apt install -y \
            qt6-base-dev \
            qt6-declarative-dev \
            qt6-multimedia-dev \
            qt6-quick3d-dev \
            libqt6opengl6-dev \
            qml6-module-qtquick \
            qml6-module-qtquick-controls \
            qml6-module-qtquick-layouts \
            qml6-module-qtmultimedia \
            qml6-module-qt3d-core \
            qml6-module-qtquick3d \
            libgstreamer1.0-dev \
            libgstreamer-plugins-base1.0-dev \
            gstreamer1.0-plugins-good \
            gstreamer1.0-plugins-bad \
            gstreamer1.0-plugins-ugly \
            gstreamer1.0-libav \
            gstreamer1.0-libcamera \
            pkg-config \
            clang"
    }
}

# Sync source files to Raspberry Pi
sync_files() {
    echo -e "${YELLOW}Syncing files to Raspberry Pi...${NC}"

    # Create project directory on RPi
    ssh_cmd "mkdir -p ${RPI_PROJECT_DIR}"

    # Sync files (excluding build directories and git)
    rsync -avz --progress \
        --exclude 'build/' \
        --exclude 'build-*/' \
        --exclude '.git/' \
        --exclude '*.o' \
        --exclude '*.a' \
        --exclude 'CMakeFiles/' \
        --exclude 'CMakeCache.txt' \
        --exclude 'dashboard' \
        "${LOCAL_PROJECT_DIR}/" \
        "${RPI_USER}@${RPI_HOST}:${RPI_PROJECT_DIR}/"

    echo -e "${GREEN}Files synced successfully${NC}"
}

# Build on Raspberry Pi
build_on_rpi() {
    echo -e "${YELLOW}Building on Raspberry Pi...${NC}"

    ssh_cmd "cd ${RPI_PROJECT_DIR} && \
        mkdir -p build && \
        cd build && \
        cmake .. -DCMAKE_BUILD_TYPE=Release && \
        make -j\$(nproc)"

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Build successful!${NC}"
        # Copy config files to build directory
        echo -e "${YELLOW}Copying config files...${NC}"
        ssh_cmd "cd ${RPI_PROJECT_DIR}/build && \
            cp -f ../config/default_config.json ./config.json 2>/dev/null || true && \
            mkdir -p config && \
            cp -f ../config/scene_config.json ./config/ 2>/dev/null || true"
    else
        echo -e "${RED}Build failed!${NC}"
        exit 1
    fi
}

# Run the application on Raspberry Pi
run_on_rpi() {
    echo -e "${YELLOW}Running dashboard on Raspberry Pi...${NC}"

    # Set display for GUI (for X11 forwarding or local display)
    ssh_cmd "cd ${RPI_PROJECT_DIR}/build && \
        export DISPLAY=:0 && \
        export QT_QPA_PLATFORM=xcb && \
        ./dashboard $*"
}

# Main menu
case "${1:-}" in
    deps)
        check_dependencies
        ;;
    sync)
        sync_files
        ;;
    build)
        sync_files
        build_on_rpi
        ;;
    run)
        shift
        run_on_rpi "$@"
        ;;
    all)
        check_dependencies
        sync_files
        build_on_rpi
        run_on_rpi
        ;;
    *)
        echo "Usage: $0 {deps|sync|build|run|all}"
        echo ""
        echo "Commands:"
        echo "  deps  - Check and install dependencies on Raspberry Pi"
        echo "  sync  - Sync source files to Raspberry Pi"
        echo "  build - Sync files and build on Raspberry Pi"
        echo "  run   - Run the application on Raspberry Pi"
        echo "  all   - Do everything: deps, sync, build, run"
        echo ""
        echo "Examples:"
        echo "  $0 all                    # Full deploy and run"
        echo "  $0 build                  # Just sync and build"
        echo "  $0 run --host 10.0.0.1    # Run with custom args"
        ;;
esac
