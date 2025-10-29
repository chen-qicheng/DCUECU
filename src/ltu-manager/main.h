#ifndef SVC_LTU_H
#define SVC_LTU_H

// 包含常用标准库头文件示例，实际需根据 svc_LTU.cpp 调整
#include <iostream>
#include <string>
#include <pthread.h>

#include "VirtualSerial.h"
#include "ModuleManEntity.h"
#include "SerialComm.h"
#include "Relay485.h"
#include "protocolGB645_2007.h"

#include "ShmInter.h"

#define MAX_RECV_LEN 128

#define LTU_DEVICE_NUM 8
#define LTU_PHASE_NUM 3

ModuleManEntity *pmod = NULL;   /// 485端口-LTU
ModuleManEntity *pmod2 = NULL;  /// 485模块-显控板

long g_LedStatusIndex = 0;

/**
 * @brief LTU数据结构体
 * @param deviceAddr 设备地址
 * @param dataIndex 数据索引
 * @param shmAddr 共享内存地址
 * @param shmSubAddr 共享内存子地址
 * @param value 读取到的值
 */
typedef struct tagLTU_Data
{  
    unsigned long long deviceAddr;
    unsigned int dataIndex;
    unsigned int shmAddr;
    unsigned char shmSubAddr;
    float value;    
} LTU_Data;


typedef struct tagLTU_All_Data
{  
    unsigned long long deviceAddr;
    unsigned int dataIndex;
    unsigned int shmAddr;
    unsigned int scale;   // 数据放大倍数
    unsigned char dateNum;
    unsigned char dateByte;
    bool isNegative; // 是否有负数
    char receiveBuf[MAX_RECV_LEN];
    float value[3];
} LTU_All_Data;


LTU_All_Data g_LTU_All_Volt[LTU_DEVICE_NUM] = 
{ 
    {0x202508090001, 0x0201ff00, 0x00510020, 10, 3, 2, false, {0}, {0,0,0}},
    {0x202508090002, 0x0201ff00, 0x00510021, 10, 3, 2, false, {0}, {0,0,0}},
    {0x202508090003, 0x0201ff00, 0x00510022, 10, 3, 2, false, {0}, {0,0,0}},
    {0x202508090004, 0x0201ff00, 0x00510023, 10, 3, 2, false, {0}, {0,0,0}},
    {0x202508090005, 0x0201ff00, 0x00510024, 10, 3, 2, false, {0}, {0,0,0}},
    {0x202508090006, 0x0201ff00, 0x00510025, 10, 3, 2, false, {0}, {0,0,0}},
    {0x202508090007, 0x0201ff00, 0x00510026, 10, 3, 2, false, {0}, {0,0,0}},
    {0x202508090008, 0x0201ff00, 0x00510027, 10, 3, 2, false, {0}, {0,0,0}},
};

LTU_All_Data g_LTU_All_Curr[LTU_DEVICE_NUM] = 
{
    {0x202508090001, 0x0202ff00, 0x00510030, 1000, 3, 3, true, {0}, {0,0,0}},
    {0x202508090002, 0x0202ff00, 0x00510031, 1000, 3, 3, true, {0}, {0,0,0}},
    {0x202508090003, 0x0202ff00, 0x00510032, 1000, 3, 3, true, {0}, {0,0,0}},
    {0x202508090004, 0x0202ff00, 0x00510033, 1000, 3, 3, true, {0}, {0,0,0}},
    {0x202508090005, 0x0202ff00, 0x00510034, 1000, 3, 3, true, {0}, {0,0,0}},
    {0x202508090006, 0x0202ff00, 0x00510035, 1000, 3, 3, true, {0}, {0,0,0}},
    {0x202508090007, 0x0202ff00, 0x00510036, 1000, 3, 3, true, {0}, {0,0,0}},
    {0x202508090008, 0x0202ff00, 0x00510037, 1000, 3, 3, true, {0}, {0,0,0}},
};

void* FansControlThread(void* arg);
void* AlarmControlThread(void* arg);



void EnableRunningLED();
void DisableRunningLED();
void EnableAlarmLED();
void DisableAlarmLED();
void EnableFan();
void DisableFan();
void EnableAlarmOutput();
void DisableAlarmOutput();
void DisableAllLEDs();
float GetTemperatureValue();


int ControlLTULEDs();
bool IsVoltageNormal(float Value);
void UpdateTMUVoltageLEDs();
void UpdateLTUVoltageLEDs();







int ReadLTUData(unsigned long long meterAddr, int index, ModuleManEntity * pModuleMan);
int ReadLTUAllData(LTU_All_Data *LTU_DataItem, ModuleManEntity * pModuleMan);


int ControlLTULEDs();
int ControlFans();
int ControlLEDGroup(int index, int status);
int SendRemoteControlCommand(int portNum, int state);
void ReadAndProcessLTUData(LTU_All_Data *LTU_DataItem);





ModuleManEntity* Create485Module(int comNum);
void Init485Module();

float GetSharedMemoryData(int shmLibrary, unsigned int shmAddr, unsigned int shmSubAddr);
int SetSharedMemoryData(LTU_Data *LTU_DataItem);
int SetAllSharedMemoryData(LTU_All_Data *LTU_DataItem);

void LogVerboseBuffer(const char *szDirect, unsigned char *pBuf, unsigned short usLen);
void ShowVoltageAndCurrent();





#endif // SVC_LTU_H
