# Willfar-DCUECU-2025

## 1. 项目概述
Willfar-DCUECU-2025 是一款面向嵌入式控制器领域的通用开发项目，核心功能包括：
- 多硬件平台适配（STM32F4/i.MX6ULL 等）
- 传感器数据采集与处理（温湿度、电流电压等）
- 工业级通信（CAN/UART/ETH）
- 低功耗控制与硬件诊断
- 适用于汽车电子、工业控制、智能设备等场景。

## 2. 目录结构
```
Willfar-DCUECU-2025/
├── build/               # 编译输出目录（自动生成，.gitignore忽略）
│   ├── bin/             # 可执行文件（.elf/.bin，用于烧录）
│   ├── obj/             # 编译中间文件（.o/.obj，无需关注）
│   └── test/            # 测试程序输出（单元/集成测试可执行文件）
├── cmake/               # CMake扩展配置
│   └── toolchain.cmake  # 交叉编译工具链配置（ARM-GCC适配）
├── CMakeLists.txt       # 根CMake配置（项目构建入口）
├── config/              # 项目配置
│   ├── hardware/        # 硬件参数（引脚定义、外设地址等，如pin_def.h）
│   └── compile/         # 编译开关（功能使能、日志级别，如compile_flags.h）
├── core/                # 核心基础模块（与业务无关，可复用）
│   ├── common/          # 公共定义（自定义类型u8/u16、错误码error_code.h）
│   └── utils/           # 工具函数（CRC校验、字节转换、超时判断）
├── docs/                # 项目文档（需自行补充）
│   ├── api/             # API接口文档（Doxygen生成）
│   ├── hardware/        # 硬件资料（原理图、MCU datasheet）
│   └── design/          # 设计文档（架构图、时序图）
├── driver/              # 硬件驱动层（按“接口+平台”拆分）
│   ├── interface/       # 驱动抽象接口（如uart_interface.h，统一调用标准）
│   └── platform/        # 平台实现（如stm32f4/uart_stm32.cpp）
├── libs/                # 第三方依赖库
│   ├── src/             # 源码库（如FreeRTOS、Protobuf）
│   └── prebuilt/        # 预编译库（如libcan.a，直接链接使用）
├── scripts/             # 自动化脚本（简化开发流程）
│   ├── build.sh         # 一键编译脚本
│   └── clean.sh         # 清理编译产物脚本
├── src/                 # 业务逻辑代码（按功能模块拆分）
│   ├── app/             # 应用入口（main函数、任务创建）
│   ├── sensor/          # 传感器业务（采集、校准、滤波）
│   ├── comm/            # 通信业务（CAN/ETH数据收发）
│   └── diag/            # 硬件诊断（电源检测、外设状态）
├── test/                # 测试代码
│   ├── unit/            # 单元测试（核心模块，如utils/crc_test.cpp）
│   └── integration/     # 集成测试（模块联调，如sensor+comm_test.cpp）
└── tools/               # 开发辅助工具（本地运行，非嵌入式端）
    └── log_analyzer/    # 日志解析工具（Python编写，分析设备输出日志）
```

## 3. 环境依赖
3.1 **软件依赖**
- 编译工具链：arm-none-linux-gnueabi-gcc
- 构建工具：CMake（≥3.10）、Make（≥4.2）
- 烧录工具：OpenOCD（支持 JTAG/SWD）或 J-Link Commander
- 可选工具：Doxygen（生成 API 文档）、VS Code（带 C/C++ 插件）
  
3.2 **硬件依赖**
- 支持的 MCU：STM32F407、i.MX6ULL（可扩展其他平台）
- 硬件版本：V2.0 及以上（需匹配config/hardware中的引脚定义）
- 调试接口：SWD（2 线）或 JTAG（4 线），供电 5V/3.3V（需按硬件要求）

## 4. 快速上手
4.1 **环境准备**
安装交叉编译工具链，并添加到系统环境变量：
```
# 示例（Linux系统）
export PATH=$PATH:/opt/arm-201305-gnueabi/bin
# 验证安装：输出版本号即成功
arm-none-eabi-gcc -v
```

克隆项目并进入根目录：
```
git clone [项目仓库地址]
cd Willfar-DCUECU-2025
```

4.2 **编译项目**
一键编译（默认编译 STM32F4 平台，可修改build.sh指定平台）：
```
# 执行编译脚本，产物输出到build/bin
./scripts/build.sh
# 如需清理编译产物，执行：
# ./scripts/clean.sh
```

编译测试程序（可选）：
```
# 编译单元测试，产物输出到build/test
cmake -DBUILD_TEST=ON ..
make -j4
```

4.3 **硬件烧录**
以 OpenOCD+SWD 为例，烧录build/bin/dcu_ecu.elf到 STM32F4：
```
# 执行烧录命令（需提前配置openocd.cfg）
openocd -f interface/jlink.cfg -f target/stm32f4x.cfg -c "program build/bin/dcu_ecu.elf verify reset exit"
```

4.4 **验证功能**
连接串口（如 UART1，波特率 115200），打开串口工具（如 minicom）：
```
minicom -b 115200 -D /dev/ttyUSB0
```

复位硬件，串口输出日志包含 “System init success” 即启动正常，可通过指令测试传感器采集、通信功能。


## 5. 详细说明
5.1 **业务模块说明**
- sensor 模块：支持 I2C/SPI 接口传感器，默认集成 SHT30（温湿度）、ADS1115（ADC），新增传感器需在src/sensor下添加驱动适配。
- comm 模块：CAN 支持 2.0B 协议（波特率 500kbps），ETH 支持 TCP 客户端模式，配置参数在config/compile/comm_config.h中修改。

5.2 **多平台适配**

如需适配新 MCU（如 ESP32）：
1. 在driver/platform下新建esp32目录，实现 GPIO/UART 等驱动；
2. 修改cmake/toolchain.cmake，添加 ESP32 的交叉编译配置；
3. 执行./scripts/build.sh esp32编译对应平台产物。


## 6. 辅助信息
6.1 **文档查看**
- API 文档：在根目录执行doxygen docs/Doxyfile，生成文档到docs/api/html，打开index.html查看。
- 硬件原理图：存放于docs/hardware目录，需联系硬件工程师获取最新版本。

6.2 **问题反馈**
- 维护团队：Jack-Chen（[chen.qicheng@qq.com]）
- 常见问题：见docs/faq.md（如 “编译报错 undefined reference to uart_send”，需检查驱动接口实现）

6.3 **许可证**
- 本项目采用 MIT 许可证，详见LICENSE文件。