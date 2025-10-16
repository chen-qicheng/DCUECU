# 启用 FetchContent 模块
include(FetchContent)

# 设置第三方库存储路径
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/third_party)

# 配置 gflags 并启用 PIC
FetchContent_Declare(
  gflags
  GIT_REPOSITORY https://github.com/gflags/gflags.git
  GIT_TAG        v2.2.2
  GIT_SHALLOW    ON
  GIT_PROGRESS   ON
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
set(GFLAGS_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gflags)


# 配置 glog（依赖 gflags）
FetchContent_Declare(
  glog
  GIT_REPOSITORY https://github.com/google/glog.git
  GIT_TAG        v0.6.0
  GIT_SHALLOW    ON
  GIT_PROGRESS   ON
)
set(WITH_GTEST OFF CACHE BOOL "" FORCE)
set(WITH_GFLAGS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glog)


# 配置 gtest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
  GIT_SHALLOW    ON
  GIT_PROGRESS   ON
)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)  # 同时构建 gmock
set(BUILD_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# 配置 nlohmann/json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    ON
  GIT_PROGRESS   ON
)
set(JSON_BuildTests OFF CACHE BOOL "Disable nlohmann/json tests" FORCE)
set(JSON_MultipleHeaders OFF CACHE BOOL "Use single-header mode" FORCE)
set(JSON_ImplicitConversions ON CACHE BOOL "Enable implicit type conversions" FORCE)
FetchContent_MakeAvailable(json)


# 配置 ZeroMQ
FetchContent_Declare(
    libzmq
    GIT_REPOSITORY https://github.com/zeromq/libzmq.git
    GIT_TAG        v4.3.5
    GIT_SHALLOW    ON
    GIT_PROGRESS   ON
)
set(ENABLE_DRAFTS           OFF CACHE BOOL "禁用性能测试" FORCE)
set(ZMQ_BUILD_TESTS         OFF CACHE BOOL "禁用单元测试" FORCE)
set(WITH_DOCS               OFF CACHE BOOL "禁用文档" FORCE)
set(WITH_PERF_TOOL          OFF CACHE BOOL "禁用性能测试工具" FORCE)
set(ZMQ_ENABLE_WS           OFF CACHE BOOL "禁用WebSocket" FORCE)
set(ZMQ_ENABLE_PGM          OFF CACHE BOOL "禁用PGM协议" FORCE)
set(ZMQ_ENABLE_TIPC         OFF CACHE BOOL "禁用TIPC协议" FORCE)
set(BUILD_SHARED_LIBS       OFF CACHE BOOL "构建静态库" FORCE)
FetchContent_MakeAvailable(libzmq)


# 配置 SQLiteCpp
FetchContent_Declare(
  SQLiteCpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG        3.3.1
  GIT_SHALLOW    ON
)
set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "禁用 SQLiteCpp 测试" FORCE)
set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "禁用 SQLiteCpp 示例" FORCE)
FetchContent_MakeAvailable(SQLiteCpp)



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
