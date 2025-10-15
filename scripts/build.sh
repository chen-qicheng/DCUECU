#!/bin/bash

# 开始时间
t1=$(date +"%Y-%m-%d %H:%M:%S:%3N")

# 项目目录
PROJECT_DIR=$(cd "$(dirname "$0")/.." && pwd)
echo "PROJECT_DIR=${PROJECT_DIR}"
# build目录
BUILD_DIR="${PROJECT_DIR}/build" 
# 工具链文件
TOOLCHAIN_FILE="${PROJECT_DIR}/config/cmake/toolchain.cmake"


# 显示帮助信息
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  all, -all, -a     Clean and build the project"
    echo "  test, -test, -t   Build and run tests"
    echo "  cross, -cross, -c Clean and cross build the project"
    echo "  clean, -clean     Clean build directory"
    echo "  help, -help, -h   Show this help message"
    echo ""
    echo "Without arguments, builds the project."
}

# 清理build文件
clean_file() {
    echo -e "Cleaning up build directory..."

    if [ -d "${BUILD_DIR}" ]; then
        rm -rf "${BUILD_DIR}" 
    fi
}


# 构建项目
build_file() {
    echo -e "\e[32m----------------------------------------------------\e[0m"
    echo -e "\e[32m               Building the project                 \e[0m"
    echo -e "\e[32m----------------------------------------------------\e[0m"

    # 生成配置并构建
    cmake -B ${BUILD_DIR} -S ${PROJECT_DIR}
    make -s -C ${BUILD_DIR} -j$(nproc)
}


# 交叉编译
cross_build() {
    echo -e "\e[32m----------------------------------------------------\e[0m"
    echo -e "\e[32m               Cross build the project              \e[0m"
    echo -e "\e[32m----------------------------------------------------\e[0m"

    # 生成配置并构建
    cmake -B ${BUILD_DIR} -S ${PROJECT_DIR} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}
    make -s -C ${BUILD_DIR} -j$(nproc)
}


# 构建并运行测试
test_file() {
    echo -e "\e[32m----------------------------------------------------\e[0m"
    echo -e "\e[32m               Testing the project                  \e[0m"
    echo -e "\e[32m----------------------------------------------------\e[0m"

    # 单元测试还没实现，先进行占位
}

####################################
if [ "$1"x = "all"x -o "$1"x = "-all"x -o "$1"x = "-a"x ]; then
    clean_file
    build_file
elif [ "$1"x = "test"x -o "$1"x = "-test"x -o "$1"x = "-t"x ]; then
    test_file()
elif [ "$1"x = "cross"x -o "$1"x = "-cross"x -o "$1"x = "-c"x ]; then
    clean_file
    cross_build
elif [ "$1"x = "clean"x -o "$1"x = "-clean"x ]; then
    clean_file
elif [ "$1"x = "help"x -o "$1"x = "-help"x -o "$1"x = "-h"x ]; then
    show_help
else
    # 不带参数编译
    build_file
fi

# 结束时间
t2=$(date +"%Y-%m-%d %H:%M:%S:%3N")

echo ""
echo "Start time: $t1"
echo "End   time: $t2"

exit 1
