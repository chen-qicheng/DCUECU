# 第三方库管理

## 概述

本项目采用 CMake 的 `FetchContent` 模块管理第三方库依赖，属于现代化依赖管理方案。该方案可自动完成第三方库的**下载、配置与构建**，确保所有开发者使用统一的依赖版本，避免版本不一致导致的问题。

-   **首次构建**：CMake 会从 GitHub 自动下载指定版本的第三方库源码，并存放在项目根目录的 `third_party` 目录下，因此首次编译耗时较长。
-   **后续构建**：已下载的源码会被复用，无需重复下载，大幅提升构建速度。

## 第三方库清单

| 库名                                               | 版本    | 功能描述                                                     | 官方仓库                             |
| -------------------------------------------------- | ------- | ------------------------------------------------------------ | ------------------------------------ |
| [gflags](https://github.com/gflags/gflags)         | v2.2.2  | 轻量级命令行参数解析库，支持多种参数类型（整数、字符串、布尔值等） | https://github.com/gflags/gflags     |
| [glog](https://github.com/google/glog)             | v0.6.0  | Google 开源日志库，支持分级日志（DEBUG/INFO/WARN/ERROR）、日志轮转、堆栈跟踪等功能 | https://github.com/google/glog       |
| [googletest](https://github.com/google/googletest) | v1.14.0 | Google 开源 C++ 测试框架，包含 Google Test（单元测试）和 Google Mock（模拟测试） | https://github.com/google/googletest |
| [nlohmann/json](https://github.com/nlohmann/json)  | v3.11.3 | 现代C++ JSON库，提供直观的接口来处理JSON数据                 | https://github.com/nlohmann/json     |
| [ZeroMQ](https://github.com/zeromq/libzmq)         | v4.3.5  | 高性能异步消息库，支持多种通信模式和协议                     | https://github.com/zeromq/libzmq     |
| [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp)| v3.3.1   | SQLite C++封装库，提供简洁的面向对象接口                     | https://github.com/SRombauts/SQLiteCpp |

## 目录结构

`third_party` 目录用于存储所有第三方库的源码、构建文件及临时文件，此目录内容无需过多关注。

## 依赖关系

 `glog` 需依赖 `gflags` 实现命令行参数配置，其他库可独立使用。


## 构建配置详情

项目已预设各第三方库的编译配置，无需手动调整，核心配置如下：

### 1. gflags 配置

```cmake
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
set(GFLAGS_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
```

### 2. glog 配置

```cmake
set(WITH_GTEST OFF CACHE BOOL "" FORCE)
set(WITH_GFLAGS ON CACHE BOOL "" FORCE)
```

### 3. googletest 配置

```cmake
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(BUILD_GTEST OFF CACHE BOOL "" FORCE)
```

其他库的配置信息请查看当前目录的 `CMakeLists.txt` 文件。
关于配置项如何确定，每个库的拥有的配置项，和配置型命名都不同。这个需要查看该库的 `CMakeLists.txt` 文件。

## 项目中使用第三方库

项目已完成第三方库的链接配置，在 `CMakeLists.txt` 中可直接通过**目标名**链接所需库，无需手动指定头文件或库文件路径：

```cmake
# 示例1：链接 gflags（命令行参数解析）
target_link_libraries(your_target PRIVATE gflags::gflags)

# 示例2：链接 glog（日志功能）
target_link_libraries(your_target PRIVATE glog::glog)

# 示例3：链接 googletest（单元测试 + 模拟测试）
target_link_libraries(your_test_target PRIVATE 
    gtest::gtest        # Google Test 核心库
    gtest::gmock        # Google Mock 模拟库
    gtest::gtest_main   # googletest 提供的默认 main 函数（可选）
)

# 示例4：链接 nlohmann/json（JSON处理）
target_link_libraries(your_target PRIVATE nlohmann_json::nlohmann_json)

# 示例5：链接 ZeroMQ（消息通信）
target_link_libraries(your_target PRIVATE libzmq)

# 示例6：链接 SQLiteCpp（数据库操作）
target_link_libraries(your_target PRIVATE SQLiteCpp)
```

## 更新第三方库

如需升级第三方库版本，按以下步骤操作：

1.  **修改版本配置**

    编辑 `/libs/third_party/CMakeLists.txt` 文件，找到对应库的 `FetchContent_Declare` 块，将 `GIT_TAG` 参数改为目标版本号（需使用 GitHub 上存在的标签，如 `v2.2.3`）。

    示例：更新 gflags 到 v2.2.3

    ```cmake
    FetchContent_Declare(
      gflags
      GIT_REPOSITORY https://github.com/gflags/gflags.git
      GIT_TAG        v2.2.3  # 原版本为 v2.2.2，此处修改为新版本
      GIT_SHALLOW    ON      # 仅下载指定版本的源码（不下载完整历史，加快速度）
    )
    ```

2.  **清理旧版本（可选但推荐）**

    删除 `third_party` 目录下对应库的文件夹（如 `gflags-src`、`gflags-build`、`gflags-subbuild`），确保 CMake 重新下载新版本源码：

    ```bash
    rm -rf third_party/gflags-*
    ```

3.  **重新构建**

    执行项目构建命令，CMake 会自动下载新版本源码并编译：

    ```bash
    cmake -B build -S .
    make -C build -j$(nproc)
    ```

    

## 故障排除

### 1. 下载失败

**现象**：CMake 构建时卡在 “Downloading...”，或提示 “Failed to download repository”。

**解决步骤**：

-   检查网络连接：确保服务器能访问 GitHub（可通过 `ping github.com` 或 `telnet github.com 443` 验证）。
-   清理缓存：删除 `third_party` 目录下对应库的 `*-subbuild` 文件夹（如 `gflags-subbuild`），重新触发下载。
-   手动下载（备用）：若网络限制无法自动下载，可手动从 GitHub 下载源码，解压到 `third_party/库名-src` 目录（如 `third_party/gflags-src`），再重新构建。

### 2. 构建错误

**现象**：编译时提示 “undefined reference”“no such file or directory” 等错误。

**解决步骤**：

-   清理构建目录：删除 `build` 目录，重新生成构建文件（避免旧配置干扰）：

    ```bash
    rm -rf build/*
    cmake -B build -S .
    ```

-   检查编译器版本：确保 GCC/G++ 版本符合第三方库要求（推荐 GCC 8.0+，可通过 `gcc --version` 查看）。

-   查看依赖是否缺失：若错误与 `glog` 相关，确认 `gflags` 已成功构建（可检查 `third_party/gflags-build` 目录是否存在 `libgflags.a`）。

### 3. 磁盘空间不足

**现象**：构建时提示 “No space left on device”。

**解决步骤**：

-   清理构建产物：删除 `build` 目录下的临时文件：

    ```bash
    rm -rf build/*
    ```

-   清理第三方库缓存：删除已下载但暂时不用的库源码（如旧版本的 `*-src` 目录）：

    ```bash
    # 示例：删除 gflags 旧版本源码
    rm -rf third_party/gflags-src
    ```

## 许可证信息

 - 所有第三方库均使用官方开源许可证，符合商业项目使用要求。
 - 各库的完整许可证文本可在其源码目录的 `LICENSE` 文件中查看（如 `third_party/gflags-src/LICENSE`）。