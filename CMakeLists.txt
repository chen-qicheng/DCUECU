cmake_minimum_required(VERSION 3.11)
project(Willfar-DCUECU-2025 VERSION 1.0.0)

set(CMAKE_BUILD_TYPE "release")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器选项
set(CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} \
    -Wall \
    -Wextra \
    -Wno-dev \
    -Wno-unused-parameter \
    -Wno-unused-function \
    -Wno-unused-variable \
    -Wno-unused-but-set-variable \
    -Wunused-includes"
)

# 设置不同类型的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

# 为中间目标文件设置输出目录
set(CMAKE_OBJ_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/obj)

# 确保输出目录存在
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build/obj)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

# 使用add_subdirectory包含子目录
add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(test)