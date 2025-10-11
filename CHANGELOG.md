# Changelog

All notable changes to the Willfar-DCUECU-2025 project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- 搭建工程目录
- 输出文件`README.md`和`CHANGELOG.md`
- 计划完成：CMAKE和交叉编译
- 计划完成：单元测试环境
- 计划完成：`build.sh`和`clean.sh`
- 计划完成：部分库文件（log、dlms协议、数据库）
- 计划完成：sensor、ltu、

### Fixed
- 待修复：高湿度环境下ADC读数漂移问题

## [1.1.0] - 2025-06-30
### Added
- 新增i.MX6ULL平台驱动适配（`driver/platform/imx6ull`），支持GPIO/UART/CAN外设
- 实现OTA远程升级功能（`src/ota`），支持通过ETH接口传输固件
- 补充硬件诊断模块（`src/diag`），可检测电源电压、外设连接状态

### Fixed
- 修复STM32F4平台CAN总线在高负载下死锁的bug（`driver/platform/stm32f4/can_stm32.cpp`）
- 解决`core/utils/crc.cpp`中CRC32计算对空数据处理错误的问题

### Changed
- 优化`scripts/build.sh`，支持指定编译平台（如`./build.sh stm32f4`或`./build.sh imx6ull`）
- 调整`config/hardware/pin_def.h`中ETH_RST引脚定义（从PC0改为PC1）

### Hardware
- 适配硬件版本V2.0（增加4G模块接口，调整电源电路）

## [1.0.0] - 2025-01-15
### Added
- 初始版本发布，支持STM32F407平台
- 实现基础驱动：GPIO（`driver/platform/stm32f4/gpio_stm32.cpp`）、UART（`driver/platform/stm32f4/uart_stm32.cpp`）、ADC（`driver/platform/stm32f4/adc_stm32.cpp`）
- 集成FreeRTOS实时操作系统（`libs/src/freertos`）
- 完成温湿度传感器数据采集业务（`src/sensor`）
- 实现基于UART的设备通信协议（`src/comm/uart_protocol.cpp`）
