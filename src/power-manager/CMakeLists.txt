# 定义项目名称（使用提取的目录名作为项目名）
get_filename_component(TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${TARGET_NAME})

# 源文件列表
set(SOURCES
    main.cpp
)

#库列表
set(LIB_LIST 
    gflags_nothreads
    glog::glog
)

# 创建可执行文件
add_executable(${TARGET_NAME} ${SOURCES})

# 设置输出目录 (使用项目根目录中定义的设置)
set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 链接库目录 (使用项目根目录中定义的设置)
target_link_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)

target_link_libraries(${TARGET_NAME} PRIVATE 
    ${LIB_LIST}
)