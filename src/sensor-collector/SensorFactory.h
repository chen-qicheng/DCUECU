#ifndef __SENSOR_FACTORY_H__
#define __SENSOR_FACTORY_H__

#include "Sensor.h"
#include <memory>
#include <map>
#include <string>


// 工厂类
class SensorHandlerFactory 
{
public:
    virtual ~SensorHandlerFactory() = default;
    virtual std::unique_ptr<SensorHandler> createSensorHandler(SensorType type) = 0;
};


// 具体工厂类
class ConcreteSensorHandlerFactory : public SensorHandlerFactory 
{
public:
    std::unique_ptr<SensorHandler> createSensorHandler(SensorType type) override;
};


// 传感器管理器（使用工厂模式）
class SensorManager 
{
private:
    std::unique_ptr<SensorHandlerFactory> m_factory;
    std::map<SensorType, std::unique_ptr<SensorHandler>> sensorHandlers;
    
public:
    SensorManager();
    void initializeSensorHandlers();
    void reinitializeHandlers();
    int ProcessSensor(SensorType key);

};

#endif // __SENSOR_FACTORY_H__