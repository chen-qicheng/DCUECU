#include "Modbus.h"
#include <string.h>
#include <algorithm>

Modbus::Modbus() {
    // 构造函数
}

Modbus::~Modbus() {
    // 析构函数
}

// CRC校验表
static const uint16_t crcTable[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

uint16_t Modbus::calculateCRC(const uint8_t* data, size_t length)  
{
    uint16_t crc = 0xFFFF;
    
    for (size_t i = 0; i < length; i++) 
    {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crcTable[index];
    }
    
    return crc;
}

bool Modbus::verifyCRC(const uint8_t* data, size_t length) 
{
    if (length < 2) 
    {
        return false;
    }
    
    uint16_t calculatedCRC = calculateCRC(data, length - 2);
    uint16_t receivedCRC = (data[length - 1] << 8) | data[length - 2];
    
    return calculatedCRC == receivedCRC;
}

// 在数据末尾添加CRC
void Modbus::appendCRC(uint8_t* data, size_t length) 
{
    uint16_t crc = calculateCRC(data, length - 2);
    data[length - 2] = crc & 0xFF;
    data[length - 1] = (crc >> 8) & 0xFF;
}

void Modbus::pushUint16(std::vector<uint8_t>& vec, uint16_t value) 
{
    vec.push_back((value >> 8) & 0xFF);  // 高字节
    vec.push_back(value & 0xFF);         // 低字节
}

uint16_t Modbus::popUint16(const std::vector<uint8_t>& vec, size_t index) 
{
    if (index + 1 >= vec.size()) 
    {
        return 0;
    }
    return (vec[index] << 8) | vec[index + 1];
}


std::vector<uint8_t> Modbus::buildReadHoldingRegistersFrame(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity) {
    std::vector<uint8_t> frame;
    
    frame.push_back(slaveAddr);          // 从设备地址
    frame.push_back(0x03);               // 功能码：读保持寄存器
    pushUint16(frame, startAddr);        // 起始地址
    pushUint16(frame, quantity);         // 寄存器数量
    
    // 预留CRC位置
    frame.push_back(0x00);
    frame.push_back(0x00);
    
    // 计算并填充CRC
    appendCRC(frame.data(), frame.size());
    
    return frame;
}

std::vector<uint8_t> Modbus::buildWriteSingleRegisterFrame(uint8_t slaveAddr, uint16_t regAddr, uint16_t value) {
    std::vector<uint8_t> frame;
    
    frame.push_back(slaveAddr);          // 从设备地址
    frame.push_back(0x06);               // 功能码：写单个寄存器
    pushUint16(frame, regAddr);          // 寄存器地址
    pushUint16(frame, value);            // 寄存器值
    
    // 预留CRC位置
    frame.push_back(0x00);
    frame.push_back(0x00);
    
    // 计算并填充CRC
    appendCRC(frame.data(), frame.size());
    
    return frame;
}

std::vector<uint8_t> Modbus::buildWriteMultipleRegistersFrame(uint8_t slaveAddr, uint16_t startAddr, const std::vector<uint16_t>& values) {
    std::vector<uint8_t> frame;
    
    frame.push_back(slaveAddr);                     // 从设备地址
    frame.push_back(0x10);                          // 功能码：写多个寄存器
    pushUint16(frame, startAddr);                   // 起始地址
    pushUint16(frame, static_cast<uint16_t>(values.size()));  // 寄存器数量
    frame.push_back(static_cast<uint8_t>(values.size() * 2)); // 字节数
    
    // 添加寄存器值
    for (const auto& value : values) {
        pushUint16(frame, value);
    }
    
    // 预留CRC位置
    frame.push_back(0x00);
    frame.push_back(0x00);
    
    // 计算并填充CRC
    appendCRC(frame.data(), frame.size());
    
    return frame;
}

bool Modbus::parseReadHoldingRegistersResponse(const std::vector<uint8_t>& response, std::vector<uint16_t>& registers) {
    // 检查最小长度
    if (response.size() < 5) {
        return false;
    }
    
    // 验证CRC
    if (!verifyCRC(response.data(), response.size())) {
        return false;
    }
    
    // 检查功能码
    if (response[1] != 0x03) {
        return false;
    }
    
    // 获取字节数
    uint8_t byteCount = response[2];
    
    // 检查响应长度是否匹配
    if (response.size() != static_cast<size_t>(byteCount + 5)) {  // 1(地址) + 1(功能码) + 1(字节数) + byteCount + 2(CRC)
        return false;
    }
    
    // 解析寄存器值
    registers.clear();
    for (size_t i = 0; i < byteCount; i += 2) {
        registers.push_back(popUint16(response, 3 + i));
    }
    
    return true;
}

bool Modbus::parseWriteSingleRegisterResponse(const std::vector<uint8_t>& response, uint16_t& addr, uint16_t& value) {
    // 检查长度
    if (response.size() != 8) {  // 1(地址) + 1(功能码) + 2(地址) + 2(值) + 2(CRC)
        return false;
    }
    
    // 验证CRC
    if (!verifyCRC(response.data(), response.size())) {
        return false;
    }
    
    // 检查功能码
    if (response[1] != 0x06) {
        return false;
    }
    
    // 解析地址和值
    addr = popUint16(response, 2);
    value = popUint16(response, 4);
    
    return true;
}

bool Modbus::parseWriteMultipleRegistersResponse(const std::vector<uint8_t>& response, uint16_t& startAddr, uint16_t& quantity) {
    // 检查长度
    if (response.size() != 8) {  // 1(地址) + 1(功能码) + 2(起始地址) + 2(数量) + 2(CRC)
        return false;
    }
    
    // 验证CRC
    if (!verifyCRC(response.data(), response.size())) {
        return false;
    }
    
    // 检查功能码
    if (response[1] != 0x10) {
        return false;
    }
    
    // 解析起始地址和数量
    startAddr = popUint16(response, 2);
    quantity = popUint16(response, 4);
    
    return true;
}