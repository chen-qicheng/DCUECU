#include "Sensor.h"
#include "SerialPort.h"
#include "Modbus.h"

#include <iostream>
#include <iomanip>

#include <unistd.h>
#include <string.h>


using namespace std;

// 初始化全局map
std::map<SensorType, SensorConfig> g_sensorConfig = 
{   //传感器类型             modubus 地址、数据项、长度      shm地址  告警阈值    告警OBIS            传感器名字
    {SensorType::WIRELESS_TEMP_A,   {0x80, 0x00, 0x02,  0x0051000903, 50, "0.96.99.98.40.255", "Wireless Temperature A"}},
    {SensorType::WIRELESS_TEMP_B,   {0x81, 0x00, 0x02,  0x0051000A03, 50, "0.96.99.98.41.255", "Wireless Temperature B"}},
    {SensorType::WIRELESS_TEMP_C,   {0x82, 0x00, 0x02,  0x0051000B03, 50, "0.96.99.98.42.255", "Wireless Temperature C"}},
    {SensorType::WIRELESS_TEMP_N,   {0x83, 0x00, 0x02,  0x0051000C03, 50, "0.96.99.98.43.255", "Wireless Temperature N"}},
    {SensorType::LEAK_CURRENT_A,    {0x86, 0x01, 0x02,  0x0051001003, 50, "0.96.99.98.41.255", "Leakage Current A"}},
    {SensorType::LEAK_CURRENT_B,    {0x87, 0x01, 0x02,  0x0051001003, 50, "0.96.99.98.42.255", "Leakage Current B"}},
    {SensorType::LEAK_CURRENT_C,    {0x88, 0x01, 0x02,  0x0051001003, 50, "0.96.99.98.43.255", "Leakage Current C"}},
    {SensorType::TEMPERATURE,       {0x71, 0x00, 0x01,  0x0051000203, 50, "0.96.99.98.1.255" , "Temperature"}},
    {SensorType::HUMIDITY,          {0x71, 0x01, 0x01,  0x0051000103, 50, "0.96.99.98.2.255" , "Humidity"}},
    {SensorType::WATER,             {0x70, 0x00, 0x01,  0x0051000603, 1,  "0.96.99.98.4.255" , "Water" }},
    {SensorType::SMOKE,             {0x72, 0x00, 0x01,  0x0051000703, 1,  "0.96.99.98.5.255" , "Smoke" }},
    {SensorType::DOOR,              {0x03, 0x00, 0x01,  0x0051000803, 1,  "0.96.99.98.3.255" , "Door" }},
};


SensorHandler::SensorHandler() 
{
}

std::string SensorHandler::GetSensorTypeName(SensorType type) 
{
    std::map<SensorType, SensorConfig>::iterator it = g_sensorConfig.find(type);

    if (it != g_sensorConfig.end()) 
    {
        return it->second.name;
    }
    else
    {
        return "Unknown";
    }
}

int SensorHandler::BuildFrame()
{ 
    m_sendBuffer = Modbus::buildReadHoldingRegistersFrame(m_deviceAddr, m_dataItem, m_dataLength);

    return m_sendBuffer.size();
}


int SensorHandler::SendAndReceive()
{
    // 打印发送缓冲区内容
    PrintBuffer("Send", m_sendBuffer);

    // 发送数据
    int writeResult = m_serialPort.send(m_sendBuffer.data(), m_sendBuffer.size());
    if (writeResult < 0) 
    {
        return -1;
    }
    
    // 接收数据（最多接收255字节）     
    m_receiveBuffer.resize(255);
    int readResult = m_serialPort.receive(m_receiveBuffer.data(), 255);
    if (readResult < 0) 
    {
        m_receiveBuffer.resize(0);
        return -1;
    }

    // 调整接收缓冲区大小以匹配实际接收长度
    m_receiveBuffer.resize(readResult);

    // 打印接收缓冲区内容
    PrintBuffer("Receive", m_receiveBuffer);

    return readResult;
}


void SensorHandler::ProcessSensorData()
{
    // 构建请求帧
    BuildFrame();
    
    // 发送并接收数据
    SendAndReceive();
    
    // 处理数据
    ProcessData();

    // 存储数据
    StoreData();

    // 发送告警
    SendAlarm();
}


int SensorHandler::SetAlarmValue(uint8_t alarmValue)
{   
    m_alarmValue = alarmValue;
    return 0;
}

int SensorHandler::GetAlarmValue()
{   
    return m_alarmValue;
}

int SensorHandler::InitConfig(SensorConfig index)
{   
    //Set Modbus Config
    m_deviceAddr = index.deviceAddr;
    m_dataItem = index.dataItem;
    m_dataLength = index.dataLength;

    //Set Shm Config
    m_shmAddr = index.shmAddr;

    //Set Alarm Config
    m_alarmOBIS = index.alarmOBIS;
    m_alarmValue = index.alarmValue;

    return 0;
}


// 辅助函数：打印缓冲区内容（十六进制格式）
void SensorHandler::PrintBuffer(const std::string& prefix, const std::vector<uint8_t>& buffer)
{
    if (buffer.empty())
    {
        std::cout << prefix << "[0] : (empty)" << std::endl;
        return;
    }

    std::cout << prefix << "[" << buffer.size() << "] : ";
    
    // 保存当前cout状态，避免影响后续输出
    std::ios_base::fmtflags flags = std::cout.flags();
    
    for (uint8_t byte : buffer) 
    {
        std::cout << std::hex 
                  << std::setw(2) 
                  << std::setfill('0')
                  << static_cast<int>(byte) << " ";
    }
    
    // 恢复cout原始状态
    std::cout.flags(flags);
    std::cout << std::endl;
}