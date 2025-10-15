#include "Sensor.h"
#include "SensorFactory.h"
#include "SerialPort.h"

#include <unistd.h>


int main(int argc, char *argv[])
{
    SerialPort& serialPort = SerialPort::getInstance();
    serialPort.open("/dev/ttyS4", 9600, 0, 8, 1);

    SensorManager Sensor;
    while (true)
    {
        const int sensorCount = static_cast<int>(SensorType::SENSOR_NUM);

        for(int i = 0; i < sensorCount; ++i)
        {
            SensorType type = static_cast<SensorType>(i);
            Sensor.ProcessSensor(type);
            
            //建议延时不低于500ms
            sleep(2);
        }
    }
    
    return 0;
}



