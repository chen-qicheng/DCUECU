#include "main.h"

int main (int argc,char *argv[])
{ 
    // 初始化glog
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;  // 将日志输出到stderr


    time_t rawtime;
    struct tm *timenow;
    struct tm *timebegin;

    int delytime =(argc > 1 )? (unsigned char) atoi(argv[ 1 ]) : 1400;

    time(&rawtime);
    timenow = localtime(&rawtime);
    LOG(INFO) << "START RUN = " << time(NULL) << ", delytime =" << delytime << ", "
              << "Day = "<< timenow->tm_year+1900 << "-" << timenow->tm_mon+1 << "-"<< timenow->tm_mday << ", "
              << "Time = " << timenow->tm_hour << ":" << timenow->tm_min << ":" << timenow->tm_sec;


    int     paramdelay        = delytime;
    int     lstkeytm          = getlastkeytime( );
    //串口6用于载波，不适用于低压集抄，需去掉，改为串口号2(红外和维护口复用)和串口号5用于485-3被抄和ESAM，。
    int     lstttyS5          = GetttySRsNumx( 5 );
    int     lstttyS2          = GetttySRsNumx( 2 );
    bool    lstbatterystatus  = judgebatteries( );

    time_t  lstoptm     = time( NULL );

    if( lstbatterystatus == false )   //上电的时候，为电池供电
    {
        delytime = paramdelay;          //终端断电延时为基准时间，如果终端从外部供电转换到电池供电,延时时间提高到3倍
    }
    managebatteries( 1 );

    do
    {
        sleep(1);
        ManageBattery( );

        if( judgebatteries() )    //终端处于外部电源供电状态
        {
            LOG(INFO) << "终端处于外部电源供电状态" ;
            if( lstbatterystatus == false )     //上一次处于电池供电状态
            {
                lstbatterystatus = true;          //
                managebatteries( 1 );             //打开电池供电，以便外部电源中断供电时，能自动切换到电池供电
            }
            continue;
        }

        if( lstbatterystatus == true )    //判断终端是否从外部供电切换到内部供电的状态
        {//供电状态切换时，初始化相应变量
            lstkeytm = getlastkeytime( );
            lstttyS5 = GetttySRsNumx( 5 );
            lstttyS2 = GetttySRsNumx( 2 );
            lstoptm  = time( NULL );
            lstbatterystatus  = false;
            delytime  = paramdelay * 3;
            //powerofftm = time( NULL );    //纪录停电时刻
        }
        if(
            lstkeytm != getlastkeytime( )  ||
            lstttyS5 != GetttySRsNumx( 5 ) ||
            lstttyS2 != GetttySRsNumx( 2 ) )
        {
            lstkeytm = getlastkeytime( );
            lstttyS5 = GetttySRsNumx( 5 );
            lstttyS2 = GetttySRsNumx( 2 );
            lstoptm  = time( NULL );
            //终端从外部电源供电到内部电池供电，开始时必须保证3倍的基准时间用于数据上报
            //停电后有操作，将最后一次操作时间作为电池供电延迟时间的起点
        }

        float BatteryVolt = 0;
        if(GetBatteryVoltage( &BatteryVolt )==false)
        {
            continue;
        }

        //printf( "LST =%ld delytime = %d>>>>>>>>>>>>\n", time( NULL ), delytime);//std::abs( time( NULL ) - lstoptm ), delytime);
        LOG(INFO) << "curtime = " << time( NULL ) <<"delytime = "<< delytime << " BatteryVolt = " << BatteryVolt;	


        if ((std::abs(time(NULL) - lstoptm) > delytime)||(BatteryVolt  < 4.3))
        { 
            timebegin = localtime(&lstoptm);
            time(&rawtime);
            timenow = localtime(&rawtime);
            
            LOG(INFO) << "lstoptm is " << lstoptm << ":s rawtime is " << rawtime;
            LOG(INFO) << "Battery supply at = " << timebegin->tm_year+1900 << ":year "
                      << timebegin->tm_mon+1 << ":month " << timebegin->tm_mday << ":day "
                      << timebegin->tm_hour << ":hour " << timebegin->tm_min << ":min "
                      << timebegin->tm_sec << ":s";
            
            LOG(INFO) << "Battery close at = " << timenow->tm_year+1900 << ":year "
                      << timenow->tm_mon+1 << ":month " << timenow->tm_mday << ":day "
                      << timenow->tm_hour << ":hour " << timenow->tm_min << ":min "
                      << timenow->tm_sec << ":s";

            LOG(INFO) << "Battery Gap is = " << std::abs(rawtime-lstoptm) << " seconds";

            managebatteries( 0 );
        
            sleep( 60 );
        }

    }while( true );

    // 关闭glog（释放资源）
    google::ShutdownGoogleLogging();
    return 0;
}

int getlastkeytime( )
{
    char keytime[ 14 ]={ 0 };
    int fd = open( "/proc/wfet/last_key_time", O_RDWR);
    read( fd, keytime, 14);
    close( fd );

    char * cputype = getenv( "cputype" );
    if ( !cputype )
    {
        char tmp[]="2410";
        cputype = tmp;
    }

    if ( 0 == strcmp( cputype, "9260" ) )
    return atoll( keytime );
    else
    return atoll( &keytime[ 8 ] );  //去除年
}

bool judgebatteries( )//判断电池状态（上电 断电）
{
    int fd;
    //  unsigned char reg;
    char bit_reg;
    fd = open("/proc/wfet/poweroff_state", O_RDWR);
    if (fd < 0)
    {
        LOG(ERROR) << "judgebatteries(): can not open poweroff_state";
        return false;
    }

    read(fd, &bit_reg, 1);
    close(fd);

    return ( bit_reg == '1' ) ? false : true ;                  //电池在上电状态
}

bool managebatteries(int flag)//控制电池(上电 断电)
{
    char * cputype = getenv( "cputype" );
    if ( !cputype )
    {
        char tmp[]="2410";
        cputype = tmp;
    }
    if ( 0 == strcmp( cputype, "9260" ) )
    {
        SetSupplyStatus( flag );
    }
    else
    {
        unsigned char writebuff[2]={0};
        if( flag == 1 )
        {
            writebuff[0] = 0xF7;//掩码 开放第4位
            writebuff[1] = 0xff;//第四位写1,表示打开电池电源
            LOG(INFO) << "打开电池电源: writebuff[0]= " << writebuff[0] << "writebuff[1]= "<< writebuff[1];//
        }
        else
        {
            writebuff[0] = 0xF7;//掩码 开放第4位
            writebuff[1] = 0x00;//第四位写0,表示关闭电池电源
            LOG(INFO) << "关闭电池电源: writebuff[0]= " << writebuff[0] << ",writebuff[1]= "<< writebuff[1];//
        }

        int fd = open("/dev/wfet1000_new_gpio", O_RDWR);
        LOG(INFO) << "managebatteries flag= " << flag;

        if (fd < 0)
        {
            LOG(ERROR) <<"managebatteries: can not open wfet1000_new_gpio";
            return false;
        }

        write(fd,writebuff,2);
        close(fd);
    }
    return true;
}

int GetttySRsNumx( char id )
{
    char temp_filestr[ 1024 ]={ 0 };

    int fd = open("/proc/tty/driver/atmel_serial",O_RDWR);
    if( fd < 0 ) return 0;

    read( fd, temp_filestr, 1024);
    close( fd );

    char tmpid[ 10 ] = { 0 };
    sprintf( tmpid, "%d:", id );


    char* pstr = strstr( temp_filestr, tmpid );
    if( pstr == NULL ) return 0;


    char *startP = strstr( pstr, "rx:" );
    if( startP == NULL ) return 0;

    int num = 0;
    sscanf( &startP[ 3 ], "%d", &num );
    return num;
}

bool GetBatteryVoltage( float *pVale )
{
    char temp[ 32 ]={ 0 };

    int fd = open("/proc/wfet/battery_adc",O_RDWR);
    if( fd < 0 ) return false;

    read( fd, temp, sizeof( temp ));
    close( fd );
    int vale = 0;
    sscanf( temp, "%x", &vale );
    *pVale = vale * 9.9 / 0x3ff;
    LOG(INFO) << "Battary Vale = " << vale << "Vol = " << *pVale;
    return true;
}

void ManageBattery( )
{
    static time_t lstgetvoltm = 0;
    static int  circle = 6;
    time_t curtm = time( NULL );

    LOG(INFO) << " Span = " << abs( curtm - lstgetvoltm ) << ", circle = " << circle;

    if( abs( curtm - lstgetvoltm ) < circle ) return; //定时器
    lstgetvoltm = curtm;

    float batteeyvol = 0;
    if( !GetBatteryVoltage( &batteeyvol ) ) return; //读电池电压

    bool  IsChargeABattery = GetChargeStatus( );

    if( batteeyvol < MINCHARGEVOL )       //电池电压低，需要充电
    {
        if( !IsChargeABattery )         ///不是充电状态，则控制充电
        {
            SetChargeStatus( 1 );
        }
        circle = 10;
    }

    //电压在5.6～5.8之间，认为充电完成，停止充电
    if( batteeyvol >  MINCHARGEVOL && batteeyvol < NOBATTRYVOL )
    { //关闭充电电源
        SetChargeStatus( 0 );
        circle = 6;
        return;
    }
}

bool GetChargeStatus( )
{
    char temp[ 16 ]={ 0 };
    int fd = open( "/proc/wfet/battery_charge", O_RDWR );
    if( fd < 0 ) return false;

    read( fd, temp, sizeof( temp ));
    close( fd );
    int vale = 0;
    sscanf( temp, "%d", &vale );
    LOG(INFO) << "ChargeStatus = " <<  vale;
    return vale;
}

void SetChargeStatus( int flags )
{
    char temp;
    if ( flags )
    {
        temp = '1';
    }
    else
    {
        temp = '0';
    }
    int fd = open( "/proc/wfet/battery_charge", O_RDWR );
    if( fd < 0 ) return;
    write( fd, &temp, sizeof( char ) );
    close( fd );
}

void SetSupplyStatus( int flags )
{
    char temp;
    if ( flags )
    {
        temp = '1';
    }
    else
    {
        temp = '0';
    }
    int fd = open( "/proc/wfet/battery_supply", O_RDWR );
    if( fd < 0 ) return;
    write( fd, &temp, sizeof( char ) );
    close( fd );
}