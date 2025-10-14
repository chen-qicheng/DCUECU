#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include "Func6513.h"
#include "Archive.h"

//141885825.13202876855991274895771
//const double IMAX = 17.67766953;
//jw==================================

#define LOG_PARAM6513 "/data1/param6513.dat"

double IMAX = 10.607;       //081024 17.67766953;
//const double IMAX = 5.303300859;
double VMAX = 622.61;
double ENERGY = 0.00000000000094045 * IMAX * VMAX;
double VRMS = 0.00000000000094045 * VMAX * VMAX;
double IRMS = 0.00000000000094045 * IMAX * IMAX;
double INMS    = 0.0000000000075236 * IMAX * IMAX;


const double FS = 32768 / 13;
const double NACC = 2520;
const double FREQ = 0.000000587;
void LogToFile( const char*logfile, char *argv,char *fmt, ... );

bool Read6513AnalogParam( )
{
    int i;
    CArchive arch( LOG_PARAM6513, "");
    
    if( !arch.GetItemValue( (char *)"VMAX", i) ) return false;

    VMAX = i / 1000.0;

    if( !arch.GetItemValue( (char *)"IMAX", i) ) return false;

    IMAX = i / 1000.0;
    
    ENERGY    = 0.00000000000094045 * IMAX * VMAX;
    VRMS    = 0.00000000000094045 * VMAX * VMAX;
    IRMS    = 0.00000000000094045 * IMAX * IMAX;
    INMS    = 0.0000000000075236 * IMAX * IMAX;
    return true;
}


bool Save6513ImaxVmax( int vmax, int imax )
{
    char TempBuf[ 1024] = { 0 };
    sprintf(TempBuf,"VMAX = %d \nIMAX = %d \n",vmax, imax);
    LogToFile( LOG_PARAM6513, (char *)"w+",TempBuf);
    return true;
}


void LogToFile( const char*logfile, char *argv,char *fmt, ... )
{
    FILE * flog;
    va_list ap;
    flog = fopen(logfile, argv);
    if(!flog)
        return;
    
    va_start( ap, fmt );
    vfprintf( flog, fmt, ap );
    va_end( ap );
    fclose(flog);
    return;
}


double CalcEnergy( long long orig )
{
    return  ( orig * ENERGY / 1000 ); //Wh->kWh
}

double CalcVrms( int orig )
{
    return sqrt( orig * VRMS * 3600 * FS / NACC );
}

double CalcIrms( int orig )
{
    return sqrt( orig * IRMS * 3600 * FS / NACC ) ;
}


double CalcInrms( int orig )
{
    return sqrt( orig * INMS * 3600 * FS / NACC ) ;
}


double CalcPower( int orig )
{
    return  ( orig * ENERGY * 3600  * NACC / FS / 1000); //W->kW
}

double CalcCosFi( int orig_w, int orig_var )
{
    return ( orig_w / sqrt( pow( orig_w, 2 ) + pow( orig_var, 2 ) ) );
}

double  CalcAngel( int orig_w )
{
    return ( orig_w * 360 / NACC );
}

double CalcAngel( int orig_w, int orig_var )
{
    if( orig_w  == 0 )
    {
        if( orig_var == 0 ) return 0;
        return ( orig_var > 0) ? 90 : 270;
    }
    
    float angel = atan( orig_var * 1.0 / orig_w );
    angel *= 57.296;     //换算成度数（ 180/pi ）
    if( orig_w > 0 && orig_var <= 0 )
    {
        angel += 360;
    }else if( orig_w < 0 )
    {
        angel = 180 + angel;
    }
    return angel;
}

double CalcFreq( int orig )
{
    return orig * FREQ;
}


unsigned char _Hex2Bcd( unsigned char hex ){
    hex = hex % 100;
    return (hex/10*16+hex%10);
};

unsigned char Hex2Bcd_2Byte( long long hex, unsigned char *bcd ){  
    hex = hex % 10000;
    bcd[1] = _Hex2Bcd( ( unsigned char )( hex / 100 ) );
    bcd[0] = _Hex2Bcd( ( unsigned char )( hex % 100 ) );
    return 2;
}
unsigned char Hex2Bcd_3Byte( long long hex, unsigned char *bcd ){
    hex=hex%1000000;
    bcd[2] = _Hex2Bcd( (unsigned char)(hex/10000) ) ;
    bcd[1] = _Hex2Bcd( (unsigned char)((hex%10000)/100) );
    bcd[0] = _Hex2Bcd( (unsigned char)((hex%10000)%100) );
    return 3;
}
unsigned char Hex2Bcd_4Byte( long long hex, unsigned char *bcd ){
    hex = hex % 100000000;
    bcd[3] = _Hex2Bcd( (unsigned char)(hex/1000000) ) ;
    bcd[2] = _Hex2Bcd( (unsigned char)((hex%1000000)/10000) );
    bcd[1] = _Hex2Bcd( (unsigned char)(((hex%1000000)%10000)/100) );
    bcd[0] = _Hex2Bcd( (unsigned char)(((hex%1000000)%10000)%100) );
    return 4;
}

unsigned char BigEndHex2Bcd_4Byte( long hex, unsigned char *bcd, double rate)
{
    unsigned char *p = (unsigned char *)&hex;
    long long tmp = *( p + 0 ) * 0x1000000 + *(p + 1 ) * 0x10000 + *( p + 2 ) * 0x100 + *(p+ 3);
    tmp = (long long)( tmp * rate);
    return Hex2Bcd_4Byte( tmp , bcd );
}
unsigned char BigEndHex2Bcd_3Byte( long hex, unsigned char *bcd, double rate )
{
    unsigned char *p = (unsigned char *)&hex;
    long long tmp = *( p + 0 ) * 0x1000000 + *(p + 1 ) * 0x10000 + *( p + 2 ) * 0x100 + *(p+ 3);
    tmp = (long long)( tmp * rate);
    return Hex2Bcd_3Byte( tmp, bcd );
}

int BigEnd2LittleEnd( unsigned long hex )
{
    unsigned char *p = (unsigned char *)&hex;
    int  tmp = *( p + 0 ) * 0x1000000 + *(p + 1 ) * 0x10000 + *( p + 2 ) * 0x100 + *(p+ 3);
    return tmp;
}

//////////////需量时间转换函数///////////////////
//    仅用于转换需量发生时间(4字节，月日时分)
//    返回time_t格式时间
time_t ConvertDmdTime( unsigned char *buf )
{
    time_t secs;
    struct tm tm_t;
    secs = time(NULL);
    localtime_r( &secs,&tm_t );
    if( buf[3]>0x12 || buf[3]<0x1 ) return 0;
    if( buf[2]>0x31 || buf[2]<0x1 ) return 0;
    if( buf[1]>0x24 ) return 0;
    if( buf[0]>0x60 ) return 0;
    tm_t.tm_sec  = 0;
    tm_t.tm_mon  = buf[3] - 1;
    tm_t.tm_mday = buf[2] ;
    tm_t.tm_hour = buf[1] ;
    tm_t.tm_min  = buf[0] ;
//    tm_t.tm_mon  = Bcd2Hex( buf[3] ) - 1;
//    tm_t.tm_mday = Bcd2Hex( buf[2] );
//    tm_t.tm_hour = Bcd2Hex( buf[1] );
//    tm_t.tm_min  = Bcd2Hex( buf[0] );

    return mktime( &tm_t );
}

/* ==========================================================
*   返回参数：    零序电流
*    读全局变量：  
*   功能说明：
*            1.    以va为+Y轴,发生逆相序时，以平衡情况计算；未发生逆相序时，以实际相角计算；
*            2.    单相电流小于启动电流时，设该相电压电流相角为0
*           3.   先求每相电压电流相角,再求电流对+X轴的夹角,再求各相电流的矢量和
            
*/

float CalNuturalCurrent(    float ia_float,float ib_float,float ic_float,//(ABC各相电流)
                            float ph_atob_float,//(AB相电压相角)
                            float ph_atoc_float,//(CA相电压相角)
                            float AngA, float AngB, float AngC
                        )
{    
#define        ANG2PI    (180/3.1416)

    float AxisX,AxisY,NuturalCurrent,temp1,temp2,temp3;
//    printf( "AngA = %f AngB =%f AngC = %f ph_atob_float = %f ph_atoc_float = %f\n", AngA, AngB, AngC,ph_atob_float,ph_atoc_float);

    AngA += 0;                                            //计算绝对电流角度
    AngB += ph_atob_float;                    //计算绝对电流角度
    AngC += ph_atoc_float;                    //计算绝对电流角度
    
    
    
    temp1 = cos( AngA / ANG2PI);
    temp2 = cos( AngB / ANG2PI);
    temp3 = cos( AngC / ANG2PI);

    
//    printf( "AngA = %f AngB =%f AngC = %f ph_atob_float = %f ph_atoc_float = %f\n", AngA, AngB, AngC,ph_atob_float,ph_atoc_float);
    
//    printf( " temp1 = %f temp2 = %f temp3 = %f \n", temp1,temp2,temp3);

    AxisX=temp1*ia_float+temp2*ib_float+temp3*ic_float;
    
//    printf( " AxisX = %f  \n", AxisX);
    AxisX=AxisX*AxisX;

    temp1=sin(AngA / ANG2PI);
    temp2=sin(AngB / ANG2PI);
    temp3=sin(AngC / ANG2PI);
    
//    printf( " temp1 = %f temp2 = %f temp3 = %f \n", temp1,temp2,temp3);

    AxisY=temp1*ia_float+temp2*ib_float+temp3*ic_float;
//    printf( " AxisY = %f  \n", AxisY);

    AxisY=AxisY*AxisY;
    NuturalCurrent=AxisX+AxisY;

    NuturalCurrent=sqrt(NuturalCurrent);//NuturalCurrent=sqart(AxisX*AxisX+AxisY*AxisY)
    
    
    
//    printf( " NuturalCurrent = %f  \n", NuturalCurrent);
    /////////////////////////////////////////////////////
    
    AngB = 2 * ph_atob_float - AngB;                    //计算绝对电流角度
    AngC = 2 * ph_atoc_float - AngC;                    //计算绝对电流角度
    
    
    
    temp1 = cos( AngA / ANG2PI);
    temp2 = cos( AngB / ANG2PI);
    temp3 = cos( AngC / ANG2PI);

    
//    printf( "AngA = %f AngB =%f AngC = %f ph_atob_float = %f ph_atoc_float = %f\n", AngA, AngB, AngC,ph_atob_float,ph_atoc_float);
    
//    printf( " temp1 = %f temp2 = %f temp3 = %f \n", temp1,temp2,temp3);

    AxisX=temp1*ia_float+temp2*ib_float+temp3*ic_float;
    
//    printf( " AxisX = %f  \n", AxisX);
    AxisX=AxisX*AxisX;

    temp1=sin(AngA / ANG2PI);
    temp2=sin(AngB / ANG2PI);
    temp3=sin(AngC / ANG2PI);
    
//    printf( " temp1 = %f temp2 = %f temp3 = %f \n", temp1,temp2,temp3);

    AxisY=temp1*ia_float+temp2*ib_float+temp3*ic_float;
//    printf( " AxisY = %f  \n", AxisY);

    AxisY=AxisY*AxisY;
    float NuturalCurrent1;
    NuturalCurrent1=AxisX+AxisY;

    NuturalCurrent1=sqrt(NuturalCurrent1);//NuturalCurrent=sqart(AxisX*AxisX+AxisY*AxisY)
    
    
    
//    printf( " NuturalCurrent1 = %f  \n", NuturalCurrent1);
    
    
    
    
    return (fabs( NuturalCurrent1 ) < fabs( NuturalCurrent )) ? NuturalCurrent1 : NuturalCurrent;
}


bool NoiseFlag(float threshold, float newData, float *last5Data, int maxPointNum)
{
    int i;
    float max= - 5000000, min= 5000000;
    if(maxPointNum<=0)
        return true;
    for(i=1;i<maxPointNum;i++)
    {
         last5Data[i-1] = last5Data[i]; /*last5Data[0] <-last5Data[1]...last5Data[3] <-last5Data[4]*/
    }

    last5Data[maxPointNum-1]=newData;
    for(i=0;i<maxPointNum;i++)
    {
        if( max < last5Data[i] )
          {
            max = last5Data[i];
          }

        if( min > last5Data[i] )
          {
            min=last5Data[i];
          }
    }

    if( fabs( max - min ) > threshold )
        return true;
    else
        return false;
}

bool CompareVMAX(float voltage)
{
    return(voltage>VMAX);
}

bool CompareIMAX(float current)
{
    return(current>IMAX);
}    
bool ComparePMAX(float power)
{
    return(power*1000.0>IMAX*VMAX);
}
