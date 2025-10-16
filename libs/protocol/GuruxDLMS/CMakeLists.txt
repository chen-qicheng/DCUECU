# 以当前最后一级目录名，作为库名
get_filename_component(TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 定义头文件搜索路径列表
set(INC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 自动查找当前目录及子目录下所有 .cpp 源文件
file(GLOB_RECURSE LIB_SRC_LIST "*.cpp")

# 创建库目标
add_library(${TARGET_NAME} SHARED ${LIB_SRC_LIST})

# 为库关联头文件路径
target_include_directories(${TARGET_NAME}
  PUBLIC
    ${INC_LIST}  # 依赖此库的目标会自动继承这些头文件路径
)
