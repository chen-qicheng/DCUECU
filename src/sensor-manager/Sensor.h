#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "SerialPort.h"
#include "ShmInter.h"

#include <memory>   // 对于 std::unique_ptr 和 std::make_unique
#include <map>      // 对于 std::map
#include <string>   // 对于 std::string
#include <cstdio>   // 对于 printf
#include <vector>

using namespace std;

enum class SensorType {
    WIRELESS_TEMP_A,
    WIRELESS_TEMP_B,
    WIRELESS_TEMP_C,
    WIRELESS_TEMP_N,
    LEAK_CURRENT_A,
    LEAK_CURRENT_B,
    LEAK_CURRENT_C,
    TEMPERATURE,
    HUMIDITY,
    WATER,
    SMOKE,
    DOOR,
    SENSOR_NUM
};

struct SensorConfig
{
    uint8_t deviceAddr;
    uint8_t dataItem;
    uint8_t dataLength;
    uint64_t shmAddr;
    uint8_t alarmValue;
    string alarmOBIS;
    string name;
}typedef SensorConfig;


// 声明全局传感器配置映射
extern std::map<SensorType, SensorConfig> g_sensorConfig;


//抽象产品类
class SensorHandler
{
public:
    SensorHandler();
    virtual ~SensorHandler() = default;
    virtual int BuildFrame();
    virtual int SendAndReceive();
    virtual int ProcessData() = 0;
    virtual int StoreData() = 0;
    virtual int SendAlarm() = 0;

    //模板函数：传感器处理的完整流程框架
    void ProcessSensorData();
    
    //获取传感器配置
    int InitConfig(SensorConfig index);
    int SetAlarmValue(uint8_t alarmValue);
    int GetAlarmValue();

    //获取传感器类型名称
    static string GetSensorTypeName(SensorType type);

protected:
    // modbus参数
    uint8_t m_deviceAddr;
    uint8_t m_dataItem;
    uint8_t m_dataLength;

    //共享内存参数
    uint64_t m_shmAddr;

    //报警参数
    string m_alarmOBIS;
    uint8_t m_alarmValue;

    // 读数据帧和响应数据缓存
    std::vector<uint8_t> m_sendBuffer;
    std::vector<uint8_t> m_receiveBuffer;

private:
    // 串口单例
    SerialPort& m_serialPort = SerialPort::getInstance();

    void PrintBuffer(const std::string& prefix, const std::vector<uint8_t>& buffer);
};


//具体产品类
class WirelessTemperatureSensor : public SensorHandler
{
public:
    WirelessTemperatureSensor();
    int ProcessData() override;
    int StoreData() override;
    int SendAlarm() override;

private:
    std::vector<uint16_t> m_registers;
};


class TemperatureHumiditySensor : public SensorHandler
{
public:
    TemperatureHumiditySensor();
    int ProcessData() override;
    int StoreData() override;
    int SendAlarm() override;

private:
    std::vector<uint16_t> m_registers;
};


class StatusSensor : public SensorHandler
{  
public:
    StatusSensor();
    int ProcessData() override;
    int StoreData() override;
    int SendAlarm() override;

private:
    std::vector<uint16_t> m_registers;
};


#endif /* __SENSOR_H__ */
