# 常见问题解答 (FAQ)

本文档汇总了使用本项目时的常见问题及解决方案，如有其他疑问，可通过 [联系我们](#联系我们) 反馈。

 
## 1. 构建与编译
### Q: 如何进行本地编译？
A: 可以使用以下命令进行本地编译：
```bash
cd scripts
./build.sh
```

## Q: 如何进行交叉编译？
A: 使用提供的交叉编译工具链进行编译：

```bash
cd scripts
./build.sh cross
```

## Q: 构建输出文件在哪里？
A: 构建输出文件位于项目根目录的build文件夹中：

- `build/bin/`: 可执行文件
- `build/lib/`: 库文件
- `build/obj/`: 中间编译文件


# 2. 项目结构
## Q: 项目的主要目录结构是怎样的？
A: 项目遵循模块化设计，主要目录包括：

- `src/`: 业务逻辑代码
- `core/`: 核心基础模块
- `driver/`: 硬件驱动层
- `config/`: 项目配置文件
- `test/`: 测试代码
- `scripts/`: 自动化脚本
详细结构请参考 README.md 文件。


# 3. 开发与测试
## Q: 如何运行单元测试？
A: 构建完成后，测试程序位于 build/test/ 目录中。可以直接运行相应的测试可执行文件。

## Q: 如何生成API文档？
A: 项目使用 Doxygen 生成API文档。在项目根目录执行以下命令：

```bash
doxygen Doxyfile
```
生成的文档位于 docs/api/ 目录中。

## Q: CMake版本过低，无法编译项目？
A: 可以使用以下命令升级CMake版本：
```
# 更新软件源
sudo apt update

# 安装新版 CMake
sudo apt install -y cmake

# 验证版本
cmake --version

```

或者手动安装cmake 3.11版本
```bash
# 下载cmake 3.11 源码包
wget https://github.com/Kitware/CMake/releases/download/v3.11/cmake-3.11.tar.gz

# 解压源码包
tar -zxvf cmake-3.11.tar.gz
cd cmake-3.11

# 配置编译选项（默认安装到 /usr/local/）
./bootstrap

# 编译（-j4 表示使用4个CPU核心加速，可根据你的CPU核心数调整）
make -j4

# 安装到系统（需要root权限）
sudo make install

```
太高级的cmake版本可能会因为g++版本问题导致编译错误，请根据实际需求选择合适的版本。


## Q: 64位系统下无法使用32位交叉编译工具链？
A: 64位系统下，可以使用以下命令安装32位交叉编译工具链：
```bash
sudo apt install libc6-dev-i386  # 32位系统兼容库（64位系统可选）
```


