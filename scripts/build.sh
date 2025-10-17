#!/bin/bash
set -eo pipefail 

startTime=$(date +"%Y-%m-%d %H:%M:%S:%3N")

PROJECT_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="${PROJECT_DIR}/build" 
TOOLCHAIN_FILE="${PROJECT_DIR}/config/cmake/toolchain.cmake"
echo "PROJECT_DIR=${PROJECT_DIR}"

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  all, -all, -a     Clean and build the project"
    echo "  cross, -cross, -c Clean and cross build the project"
    echo "  test, -test, -t   Build and run tests"
    echo "  clean, -clean     Clean build directory"
    echo "  cmake, -cmake     Generate build files"
    echo "  make, -make       Build the project"
    echo "  help, -help, -h   Show this help message"
    echo ""
    echo "Without arguments, builds the project."
}

show_title() {
    echo -e "\e[32m----------------------------------------------------\e[0m"
    echo -e "\e[32m               $1              \e[0m"
    echo -e "\e[32m----------------------------------------------------\e[0m"
}

cmake_project() {
    cmake -B ${BUILD_DIR} -S ${PROJECT_DIR}
}

cmake_cross_project() {
    cmake -B ${BUILD_DIR} -S ${PROJECT_DIR} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}
}

make_project() {
    make -s -C ${BUILD_DIR} -j$(nproc)
}

clean_file() {
    show_title "Cleaning up build directory"
    
    if [ -d "${BUILD_DIR}" ]; then
        rm -rf "${BUILD_DIR}" 
    fi
}

build_project() {
    show_title "Building the project"
    cmake_project
    make_project
}

build_cross_project() {
    show_title "Cross build the project"
    cmake_cross_project
    make_project
}

build_test(){
    show_title "Building and testing the project"
    # TODO
}


case "$1" in
    all|-all|-a)
        clean_file
        build_project
        ;;
    cross|-cross|-c)
        clean_file
        build_cross_project
        ;;
    test|-test|-t)
        build_test
        ;;
    clean|-clean)
        clean_file
        ;;
    cmake|-cmake)
        cmake_project
        ;;
    make|-make)
        make_project
        ;;
    help|-help|-h)
        show_help
        exit 0
        ;;
    "")  # 无参数默认构建
        make_project
        ;;
    *)
        echo -e "${RED}Error: Unknown option '$1'${RESET}"
        show_help
        exit 1
        ;;
esac

endTime=$(date +"%Y-%m-%d %H:%M:%S:%3N")

echo ""
echo "Start time: $startTime"
echo "End   time: $endTime"

exit 1
