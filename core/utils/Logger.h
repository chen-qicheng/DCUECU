#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <string>
#include <cstdint>

// 日志级别枚举（与 glog 对应，简化调用）
enum class LogLevel {
    INFO,    // 信息级日志
    WARNING, // 警告级日志
    ERROR,   // 错误级日志
    FATAL    // 致命错误级日志（会终止程序）
};

/**
 * @brief 日志工具类（封装 glog，提供项目统一的日志接口）
 */
class Logger {
public:
    /**
     * @brief 初始化日志系统
     * @param program_name 程序名称（用于日志文件名前缀）
     * @param log_dir 日志输出目录（默认："./logs"）
     * @param max_log_size_mb 单个日志文件最大大小（MB，默认：100MB）
     * @param stderr_level 输出到 stderr 的最低日志级别（默认：ERROR）
     */
    static void Init(const std::string& program_name,
                     const std::string& log_dir = "./logs",
                     size_t max_log_size_mb = 100,
                     LogLevel stderr_level = LogLevel::ERROR);

    /**
     * @brief 输出日志（通用接口）
     * @param level 日志级别
     * @param file 调用日志的文件（通常用 __FILE__ 传入）
     * @param line 调用日志的行号（通常用 __LINE__ 传入）
     * @param format 日志格式字符串（类似 printf）
     * @param ... 格式参数
     */
    static void Log(LogLevel level,
                    const char* file,
                    int line,
                    const char* format,
                    ...);

    // 禁用拷贝构造和赋值（单例语义，无需实例化）
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    // 私有构造函数，禁止实例化（所有接口为静态）
    Logger() = default;

    // 转换 LogLevel 到 glog 内部级别
    static int ConvertToGlogLevel(LogLevel level);
};

// 日志宏定义（简化调用，自动传入文件名和行号）
#define LOG_INFO(format, ...) \
    Logger::Log(LogLevel::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_WARNING(format, ...) \
    Logger::Log(LogLevel::WARNING, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    Logger::Log(LogLevel::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_FATAL(format, ...) \
    Logger::Log(LogLevel::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif  // UTILS_LOGGER_H