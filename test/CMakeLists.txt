# 添加测试可执行文件
add_executable(unit_tests
    unit/test.cpp
    
)

# 链接必要的库
target_link_libraries(unit_tests
    GTest::gtest_main
)

# 指定头文件搜索路径
target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../driver/ECU
)

# 注册测试
include(GoogleTest)
gtest_discover_tests(unit_tests)