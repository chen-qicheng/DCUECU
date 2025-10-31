# 工具链文件 - 用于交叉编译
# 使用方法: cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain.cmake

# 设置目标系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 设置交叉编译工具路径
# 注意：请根据实际安装的工具链调整这些路径
set(TOOLCHAIN_DIR /opt/arm-201305-gnueabi)
set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/libc)

set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-none-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-none-linux-gnueabi-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/bin/arm-none-linux-gnueabi-gcc)
set(CMAKE_STRIP ${TOOLCHAIN_DIR}/bin/arm-none-linux-gnueabi-strip)

# 设置目标环境路径
# 如果您有目标文件系统，请取消注释并设置正确的路径ss
# set(CMAKE_FIND_ROOT_PATH /path/to/target/rootfs)

# 设置查找策略
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 设置编译标志
# set(CMAKE_C_FLAGS "-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard" CACHE STRING "" FORCE)
# set(CMAKE_CXX_FLAGS "-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard" CACHE STRING "" FORCE)

# 设置不带前缀的程序名，以便在主机上运行
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# 其他有用的变量
set(CMAKE_POSITION_INDEPENDENT_CODE ON)