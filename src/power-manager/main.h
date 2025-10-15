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

#include "glog/logging.h"

#define LOWBATTERYVOL   4.0  //4.9
#define LOWVOLBACKUP    4.0  //4.95
#define NOBATTRYVOL     5.8     ////充电时如果没有电池，则电压最小为6.0v
#define MINCHARGEVOL    5.6       //最小充电电压，电池电压小于某一数值时启动充电

bool judgebatteries();
bool managebatteries(int flag);
bool GetBatteryVoltage( float *pVale );
bool GetChargeStatus( );
void SetChargeStatus( int flags );
void SetSupplyStatus( int flags );
void ManageBattery( );
int  getlastkeytime( );
int GetttySRsNumx( char id );


#endif // !__MAIN_H__
