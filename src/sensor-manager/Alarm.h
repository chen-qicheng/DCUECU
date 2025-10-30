#ifndef __ALARM_H__
#define __ALARM_H__

#include <string.h>
#include "GXDLMSNotify.h"
#include "GXDLMSClock.h"
#include "GXDLMSData.h"

using namespace std;

class SensorAlarm
{
public:
	// 公有静态方法，提供全局访问点
    static SensorAlarm& GetInstance() 
	{
        return instance;
    }

    
    int SendPacketToUplink(unsigned char *buff, unsigned int len);
    void ReceiveUplinkMessageQueue();
    void SendSensorEventNotification(string obis, CGXDLMSVariant eventValue);


private:
	// 私有静态实例，类加载时初始化
	static SensorAlarm instance;

	// 私有构造函数，防止外部实例化
	SensorAlarm();

	// 禁用拷贝构造和赋值操作
	SensorAlarm(const SensorAlarm&) = delete;
	SensorAlarm& operator=(const SensorAlarm&) = delete;

    int m_messageQueueIdUplinkSend = 0;
    int m_messageQueueIdUplinkReceive = 0;

    int InitializeMessageQueue();

};



#endif // !__ALARM_H__
