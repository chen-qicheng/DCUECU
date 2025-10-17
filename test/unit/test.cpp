#include <gtest/gtest.h>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>  // 补充fcntl头文件
#include "DeviceFileHandler.h"

// 测试Fixture：创建临时文件并自动清理
class DeviceFileHandlerTest : public ::testing::Test {
protected:
    std::string temp_file_;  // 临时设备文件路径

    // 测试前：创建临时文件
    void SetUp() override {
        char temp_path[] = "/tmp/test_device_XXXXXX";
        int fd = mkstemp(temp_path);  // 创建唯一临时文件
        ASSERT_NE(fd, -1) << "Failed to create temp file";
        close(fd);
        temp_file_ = temp_path;
    }

    // 测试后：删除临时文件
    void TearDown() override {
        if (!temp_file_.empty()) {
            unlink(temp_file_.c_str());
        }
    }
};

// 测试1：构造函数打开文件（有效/无效路径）
TEST_F(DeviceFileHandlerTest, ConstructorOpensFile) {
    DeviceFileHandler valid_handler(temp_file_.c_str());
    EXPECT_TRUE(valid_handler.IsOpen());
    EXPECT_GE(valid_handler.GetFd(), 0);

    DeviceFileHandler invalid_handler("/invalid/non_existent_device");
    EXPECT_FALSE(invalid_handler.IsOpen());
    EXPECT_EQ(invalid_handler.GetFd(), -1);
}

// 测试2：Open() 方法重新打开文件
TEST_F(DeviceFileHandlerTest, OpenMethod) {
    DeviceFileHandler handler(temp_file_.c_str());
    int initial_fd = handler.GetFd();
    ASSERT_TRUE(handler.IsOpen());

    // 重新打开同一文件，fd应变化
    EXPECT_TRUE(handler.Open());
    EXPECT_NE(handler.GetFd(), initial_fd);

    // 关闭后重新打开
    handler.~DeviceFileHandler();  // 手动关闭
    EXPECT_FALSE(handler.IsOpen());
    EXPECT_TRUE(handler.Open());
    EXPECT_GE(handler.GetFd(), 0);
}

// 测试3：GetData() 读取数据（正常/边界/异常）
TEST_F(DeviceFileHandlerTest, GetData) {
    // 写入测试数据
    const std::string test_data = "hello_device";
    std::ofstream(temp_file_) << test_data;

    DeviceFileHandler handler(temp_file_.c_str());
    ASSERT_TRUE(handler.IsOpen());

    // 正常读取
    char buffer[32] = {0};
    EXPECT_EQ(handler.GetData(buffer, sizeof(buffer)), test_data.size());
    EXPECT_STREQ(buffer, test_data.c_str());

    // 缓冲区不足
    char small_buf[6] = {0};
    EXPECT_EQ(handler.GetData(small_buf, sizeof(small_buf)), 5);  // 5字节+终止符
    EXPECT_STREQ(small_buf, "hello");

    // 无效参数
    EXPECT_EQ(handler.GetData(nullptr, 10), -1);
    EXPECT_EQ(handler.GetData(buffer, 0), -1);

    // 文件关闭状态
    handler.~DeviceFileHandler();
    EXPECT_EQ(handler.GetData(buffer, sizeof(buffer)), -1);
}

// 测试4：SetData() 写入数据（正常/异常）
TEST_F(DeviceFileHandlerTest, SetData) {
    DeviceFileHandler handler(temp_file_.c_str());
    ASSERT_TRUE(handler.IsOpen());

    // 正常写入
    const std::string write_data = "test_write";
    EXPECT_EQ(handler.SetData(write_data.c_str(), write_data.size()), write_data.size());

    // 验证写入结果
    std::ifstream ifs(temp_file_);
    std::string content((std::istreambuf_iterator<char>(ifs)), {});
    EXPECT_EQ(content, write_data);

    // 无效参数
    EXPECT_EQ(handler.SetData(nullptr, 10), -1);
    EXPECT_EQ(handler.SetData(write_data.c_str(), 0), -1);

    // 文件关闭状态
    handler.~DeviceFileHandler();
    EXPECT_EQ(handler.SetData(write_data.c_str(), write_data.size()), -1);
}

// 测试5：析构函数关闭文件
TEST_F(DeviceFileHandlerTest, DestructorClosesFile) {
    DeviceFileHandler* handler = new DeviceFileHandler(temp_file_.c_str());
    int fd = handler->GetFd();
    ASSERT_GE(fd, 0);

    // 析构前文件应打开
    EXPECT_NE(fcntl(fd, F_GETFD), -1);

    // 析构后文件应关闭
    delete handler;
    EXPECT_EQ(fcntl(fd, F_GETFD), -1);
}