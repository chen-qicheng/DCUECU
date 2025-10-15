#include "Sensor.h"
#include "Modbus.h"
#include "Alarm.h"
#include <iostream>

WirelessTemperatureSensor::WirelessTemperatureSensor()
{

}


int WirelessTemperatureSensor::ProcessData()
{
    if (m_receiveBuffer.empty()) 
    {
        return -2; // 接收缓冲区为空
    }

    m_registers.resize(0);
    // 解析接收到的Modbus响应
    if (Modbus::parseReadHoldingRegistersResponse(m_receiveBuffer, m_registers)) 
    {
        std::cout << "Parsed " << m_registers.size() << " registers" << std::endl;

        for(size_t i = 0; i < m_registers.size(); i++)
        {
            std::cout << "Value: " << m_registers[i] << std::endl;
        }
    } 
    else 
    {
        std::cerr << "Failed to parse Modbus response" << std::endl;
        return -1; // 解析失败
    }

    return 0;
}

int WirelessTemperatureSensor::StoreData()
{
    if(m_registers.size() < 1) 
    {
        return -1;
    }

    uint32_t mainItem;
    uint8_t subitem;

    int len = 4;
    int tempValue = m_registers[0];

    CShmIO shm;
    shm.Init(SHM_TERM_DATA, 0, RWRITE);

    mainItem = (m_shmAddr>>8) & 0xFFFFFFFF;
    subitem = m_shmAddr & 0xFF;

    return shm.SetItem((void *)&tempValue, len, mainItem, subitem);
}  

int WirelessTemperatureSensor::SendAlarm()
{
    if(m_registers.size() < 1) 
    {
        return -1;
    }

    SensorAlarm &alarm = SensorAlarm::GetInstance();

    if(m_registers[0] > m_alarmValue)
    {
        std::cout << "Alarm triggered: " << m_registers[0] << std::endl;
        alarm.SendSensorEventNotification(m_alarmOBIS, CGXDLMSVariant(m_registers[0]));
    }

    return 0;
}