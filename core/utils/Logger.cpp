#include "Logger.h"
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <cstdio>
#include <cstdarg>
#include <sys/stat.h>
#include <unistd.h>

// 转换自定义 LogLevel 到 glog 级别（内部实现）
int Logger::ConvertToGlogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:    return google::GLOG_INFO;
        case LogLevel::WARNING: return google::GLOG_WARNING;
        case LogLevel::ERROR:   return google::GLOG_ERROR;
        case LogLevel::FATAL:   return google::GLOG_FATAL;
        default:                return google::GLOG_INFO;
    }
}

// 初始化日志系统
void Logger::Init(const std::string& program_name,
                  const std::string& log_dir,
                  size_t max_log_size_mb,
                  LogLevel stderr_level) {
    // 初始化 glog（传入程序名，用于日志前缀）
    google::InitGoogleLogging(program_name.c_str());

    // 创建日志目录（若不存在）
    struct stat dir_stat;
    if (stat(log_dir.c_str(), &dir_stat) == -1) {
        // 递归创建目录（0755 权限：所有者读写执行，组和其他读执行）
        std::string cmd = "mkdir -p " + log_dir;
        system(cmd.c_str());
    }

    // 设置日志输出目录（INFO 及以上级别均输出到该目录）
    std::string log_prefix = log_dir + "/" + program_name;
    google::SetLogDestination(google::GLOG_INFO, log_prefix.c_str());

    // 设置单个日志文件最大大小（MB）
    FLAGS_max_log_size = max_log_size_mb;

    // 设置 stderr 输出的最低级别（默认只输出 ERROR 及以上）
    google::SetStderrLogging(ConvertToGlogLevel(stderr_level));

    // 启用日志颜色（若终端支持）
    FLAGS_colorlogtostderr = true;

    // 安装崩溃信号处理器（程序崩溃时输出堆栈信息到日志）
    google::InstallFailureSignalHandler();

    // 输出初始化成功日志
    LOG_INFO("Logger initialized. Log directory: %s, Max size per file: %zu MB",
             log_dir.c_str(), max_log_size_mb);
}

// 日志输出实现（封装 glog 的 VLOG 接口）
void Logger::Log(LogLevel level,
                 const char* file,
                 int line,
                 const char* format,
                 ...) {
    // 处理可变参数列表
    va_list args;
    va_start(args, format);

    // 根据级别调用 glog 的日志接口
    int glog_level = ConvertToGlogLevel(level);
    google::LogMessage(file, line, glog_level).stream() << google::LogMessage::va_listToString(format, args);

    va_end(args);
}