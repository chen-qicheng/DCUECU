#include "Sensor.h"
#include "SensorFactory.h"
#include <iostream>


/**
 * @brief 具体工厂类实现，用于创建各种传感器处理器
 * @param type 传感器类型枚举值
 * @return 对应类型的传感器处理器唯一指针
 */
std::unique_ptr<SensorHandler> ConcreteSensorHandlerFactory::createSensorHandler(SensorType type)
{
    switch (type)
    {
    case SensorType::WIRELESS_TEMP_A:
    case SensorType::WIRELESS_TEMP_B:
    case SensorType::WIRELESS_TEMP_C:
    case SensorType::WIRELESS_TEMP_N:
    case SensorType::LEAK_CURRENT_A:
    case SensorType::LEAK_CURRENT_B:
    case SensorType::LEAK_CURRENT_C:
        return std::unique_ptr<SensorHandler>(new WirelessTemperatureSensor());
        break;

    case SensorType::TEMPERATURE:
    case SensorType::HUMIDITY:
        return std::unique_ptr<SensorHandler>(new TemperatureHumiditySensor());
        break;

    case SensorType::WATER:
    case SensorType::DOOR:
    case SensorType::SMOKE:
        return std::unique_ptr<SensorHandler>(new StatusSensor());
        break;

    default:
        return nullptr; 
    }
}


/**
 * @brief 传感器管理器构造函数
 *        初始化工厂对象并调用初始化函数
 */
SensorManager::SensorManager() 
{
    std::cout << "SensorManager::SensorManager()" << std::endl;

    m_factory = std::unique_ptr<SensorHandlerFactory>(new ConcreteSensorHandlerFactory());

    initializeSensorHandlers();
}


/**
 * @brief 初始化所有传感器处理器
 *        使用工厂模式创建各种类型的传感器处理器并存储在映射表中
 */
void SensorManager::initializeSensorHandlers()
{
    const int sensorCount = static_cast<int>(SensorType::SENSOR_NUM);
    for(int i = 0; i < sensorCount; ++i)
    {
        SensorType type = static_cast<SensorType>(i);

        string sensorName = SensorHandler::GetSensorTypeName(type);
        std::cout << "Initialize sensor handler for type: " << sensorName << std::endl;

        sensorHandlers[type] = m_factory->createSensorHandler(type);

        sensorHandlers[type]->InitConfig(g_sensorConfig[type]);
    }
}


/**
 * @brief 处理传感器数据
 *        查找并处理特定类型的传感器数据
 * @return 处理结果，0表示成功
 */
int SensorManager::ProcessSensor(SensorType key)
{

    for(std::map<SensorType, std::unique_ptr<SensorHandler>>::iterator it = sensorHandlers.begin(); it != sensorHandlers.end(); ++it)
    {
        if (it->first == key)
        {
            SensorHandler* handler = it->second.get();
            if (handler)
            {
                std::string sensorName = handler->GetSensorTypeName(key);
                
                std::cout << std::endl;
                std::cout << "Sensor: " << sensorName << std::endl;

                handler->ProcessSensorData();  
            }
        }
    }

    return 0;
}


/**
 * @brief 重新初始化传感器处理器
 *        清空现有处理器并重新创建所有处理器实例
 */
void SensorManager::reinitializeHandlers()
{
    // 重新初始化传感器处理程序
    sensorHandlers.clear();
    initializeSensorHandlers();
}