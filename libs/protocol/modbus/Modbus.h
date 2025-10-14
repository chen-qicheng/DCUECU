#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <cstdint>
#include <vector>
#include <cstddef>

class Modbus {
public:
    Modbus();
    ~Modbus();

    // CRC校验相关函数
    static uint16_t calculateCRC(const uint8_t* data, size_t length);
    static bool verifyCRC(const uint8_t* data, size_t length);
    static void appendCRC(uint8_t* data, size_t length);

    // Modbus帧构建函数
    static std::vector<uint8_t> buildReadHoldingRegistersFrame(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity);
    static std::vector<uint8_t> buildWriteSingleRegisterFrame(uint8_t slaveAddr, uint16_t regAddr, uint16_t value);
    static std::vector<uint8_t> buildWriteMultipleRegistersFrame(uint8_t slaveAddr, uint16_t startAddr, const std::vector<uint16_t>& values);

    // Modbus响应解析函数
    static bool parseReadHoldingRegistersResponse(const std::vector<uint8_t>& response, std::vector<uint16_t>& registers);
    static bool parseWriteSingleRegisterResponse(const std::vector<uint8_t>& response, uint16_t& addr, uint16_t& value);
    static bool parseWriteMultipleRegistersResponse(const std::vector<uint8_t>& response, uint16_t& startAddr, uint16_t& quantity);

private:
    // 辅助函数
    static void pushUint16(std::vector<uint8_t>& vec, uint16_t value);
    static uint16_t popUint16(const std::vector<uint8_t>& vec, size_t index);
};

#endif // !__MODBUS_H__


