#include "Alarm.h"

#include "MessageQue.h"
#include "ipc_resource.h"

#include <string.h>
#include <sys/msg.h>


// 初始化静态成员
SensorAlarm SensorAlarm::instance;

SensorAlarm::SensorAlarm()
{
    InitializeMessageQueue();
}



/**
 * @brief 初始化消息队列
 * @return 成功返回0，失败返回-1
 */
int SensorAlarm::InitializeMessageQueue()
{
    /* 创建Service和gprs进程之间通信消息队列 */
    m_messageQueueIdUplinkSend = msgget(MSG_SERVICE_TO_GPRS, 0666 | IPC_CREAT);
    if (-1 == m_messageQueueIdUplinkSend)
    {
        std::cout << "create message queue failed" << std::endl;
        return -1;
    }

    m_messageQueueIdUplinkReceive = msgget(MSG_GPRS_TO_SERVICE, 0666 | IPC_CREAT);
    if (-1 == m_messageQueueIdUplinkReceive)
    {
        std::cout << "create message queue failed" << std::endl;
        return -1;
    }

    return 0;
}

/**
 * @brief 发送数据包到上行链路
 * @param buff 数据缓冲区
 * @param len 数据长度
 */
int SensorAlarm::SendPacketToUplink(unsigned char *buff, unsigned int len)
{
    my_msgbuf msg_buf = {0};
    //此处消息类型需为1，不然gprs进程收不到
    msg_buf.msgtype = 1;
    if(len > M_MSG_LEN)
    {
        return -1;
    }
    memcpy(msg_buf.pbuf, buff, len);

    //发送的内容必须是msg_buf结构,但长度是msg_buf.pbuf装载内容的长度
    int ret = msgsnd(m_messageQueueIdUplinkSend, &msg_buf, len, IPC_NOWAIT);
    if (ret < 0)
    {
        return -2;
    }
    
    return 0;
}

/**
 * @brief 接收上行链路消息队列
 */
void SensorAlarm::ReceiveUplinkMessageQueue()
{
    my_msgbuf msg_buf = {0};

    int ret = msgrcv(m_messageQueueIdUplinkReceive, &msg_buf, M_MSG_LEN, 0, IPC_NOWAIT);
    if (ret <= 0)
    {
        std::cout << "ReceiveUplinkMessageQueue ret=" << ret << std::endl;
        return;
    }
}


/**
 * @brief 发送传感器事件通知
 * 
 * 该函数用于构造并发送一个传感器事件的通知消息。它会生成包含时钟时间和事件值的DLMS通知，
 * 并将这些数据通过网络发送出去。
 * 
 * @param obis 传感器对象的OBIS码，用于标识特定的传感器或数据点。
 * @param eventValue 事件值，表示传感器检测到的具体事件内容。
 */
void SensorAlarm::SendSensorEventNotification(string obis, CGXDLMSVariant eventValue)
{
    // 创建DLMS通知对象
    CGXDLMSNotify notifyObj(true, 1, 1, DLMS_INTERFACE_TYPE_WRAPPER);

    // 获取当前时间并设置到时钟对象中
    CGXDLMSClock clock;
    CGXDateTime now = CGXDateTime::Now();
    clock.SetTime(now);

    // 设置事件代码值
    CGXDLMSData eventCode;
    eventCode.SetValue(eventValue);

    // 创建通用档案对象，使用传入的OBIS码进行初始化
    CGXDLMSProfileGeneric pgObject(obis);

    std::vector<CGXByteBuffer> notificationBuffers;
    std::vector<std::pair<CGXDLMSObject*, unsigned char>> notificationObjects;

    // 将时钟对象和事件代码对象添加到通知对象列表中
    notificationObjects.push_back(std::make_pair(&clock, 2));
    notificationObjects.push_back(std::make_pair(&eventCode, 2));

    // 生成事件通知消息
    int result = notifyObj.GenerateEventNotificationMessages(&pgObject, 2, NULL, notificationObjects, notificationBuffers);
    if (result != 0)
    {
        printf("GenerateEventNotificationMessages failed with error code: %d\n", result);
        return;
    }

    // 遍历生成的通知缓冲区，并逐个发送数据包
    for(std::vector<CGXByteBuffer>::iterator it = notificationBuffers.begin(); it != notificationBuffers.end(); ++it)
    {
        cout << "Send Sensor Event Notification: " << it->ToHexString() << endl;
        SendPacketToUplink(it->GetData(), it->GetSize());
    }
}


