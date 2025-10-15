# 启用 FetchContent 模块
include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/third_party)


# 配置 gflags 并启用 PIC
FetchContent_Declare(
  gflags
  GIT_REPOSITORY https://github.com/gflags/gflags.git
  GIT_TAG        v2.2.2
  GIT_SHALLOW    ON
)
set(GFLAGS_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
set(GFLAGS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gflags)


# 配置 glog（依赖 gflags）
FetchContent_Declare(
  glog
  GIT_REPOSITORY https://github.com/google/glog.git
  GIT_TAG        v0.6.0
  GIT_SHALLOW    ON
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(WITH_GFLAGS ON CACHE BOOL "" FORCE)
set(gflags_DIR "${gflags_BINARY_DIR}" CACHE PATH "" FORCE)
FetchContent_MakeAvailable(glog)


# 配置 gtest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
  GIT_SHALLOW    ON
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)  # 同时构建 gmock
set(BUILD_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# # 示例：创建测试目标
# add_executable(my_test test.cpp)
# # 链接 gtest、gmock 和主程序依赖
# target_link_libraries(my_test PRIVATE
#   gtest::gtest
#   gtest::gmock
#   gtest::gtest_main  # 提供默认 main 函数（可选）
#   glog::glog
# )

# # 注册测试（可选，用于 ctest 运行）
# include(GoogleTest)
# gtest_discover_tests(my_test)
