#ifndef __MAIN_H__
#define __MAIN_H__

#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>

#include "gflags/gflags.h"
#include "glog/logging.h"

#define MIN_BATTERY_POWERED_VOLTAGE   4.0
#define MIN_CHARGING_VOLTAGE    5.6       //最小充电电压
#define MAX_CHARGING_VOLTAGE    5.8


void GlogInit();
void GflagsInit(int argc, char* argv[]);
void DeviceInit();

void* PowerControlThread(void* arg);
void ControlBatteryPower();
void* ChargingControlThread(void* arg);
void ControlBatteryCharging();
void* KeyMonitorThread(void* arg);

bool IsJustSwitchedToExternalPower();
bool IsLongTimeNoKeyPressed();
bool IsBatteryVoltageLow();
bool IsMinChargingVoltageReached();
bool IsMaxChargingVoltageReached();

void EnableCharging();
void DisableCharging();

int GetLastKeyTime();
bool IsBatteryPowered();
bool IsExternalPowered();

bool EnableBatteryPowered();
bool DisableBatteryPowered();
float GetBatteryVoltage();
bool IsCharging();

void SetChargeStatus(int enable);



class DeviceFileHandler
{ 
    public:
    DeviceFileHandler() : m_fd(-1){}
    DeviceFileHandler(const char * fileName );
    ~DeviceFileHandler();

    bool Open();
    bool IsOpen();
    int GetFd();
    int GetData( char * data, int size );
    int SetData( const char * data, int size );

    private:
        std::string m_fileName;
        int m_fd;
};


#endif // !__MAIN_H__
