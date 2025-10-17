#include "main.h"

#include <pthread.h>
#include <signal.h>

DEFINE_int32(keyDelayTime, 1400, "key delay time in seconds");
int keyDelayTime;
int lastKeyPressedTime = 0;

// 线程相关变量
pthread_t power_control_thread;
pthread_t charging_control_thread;
pthread_t key_monitor_thread;
bool threads_running = true;

int main (int argc,char *argv[])
{ 
    GflagsInit(argc, argv);
    GlogInit();
    DeviceInit();

    LOG(INFO) << "start create threads...";
    pthread_create(&power_control_thread, NULL, PowerControlThread, NULL);
    pthread_create(&charging_control_thread, NULL, ChargingControlThread, NULL);
    pthread_create(&key_monitor_thread, NULL, KeyMonitorThread, NULL);

    LOG(INFO) << "start main loop...";
    while(true) 
    {
        sleep(1);
    }

    LOG(INFO) << "exit the program...";
    threads_running = false;
    pthread_join(power_control_thread, NULL);
    pthread_join(charging_control_thread, NULL);
    pthread_join(key_monitor_thread, NULL);

    google::ShutdownGoogleLogging();
    gflags::ShutDownCommandLineFlags();
    
    return 0;
}

void GlogInit()
{
    google::InitGoogleLogging("power-manager");

    std::string log_dir = "./glog_logs/";
    struct stat dir_stat;
    if (stat(log_dir.c_str(), &dir_stat) == -1) {
        mkdir(log_dir.c_str(), 0755);
    }

    google::SetLogDestination(google::GLOG_INFO, log_dir.c_str());
    google::EnableLogCleaner(30);
    FLAGS_max_log_size = 10;

    google::SetStderrLogging(google::GLOG_ERROR);
    google::InstallFailureSignalHandler();
}

void GflagsInit(int argc, char* argv[])
{
    gflags::SetUsageMessage(
        "Power management program for device: "
        "handles battery power, charging control and key monitoring."
    );
    gflags::ParseCommandLineFlags(&argc, &argv, true);
}

void DeviceInit()
{
    keyDelayTime = FLAGS_keyDelayTime;
    EnableBatteryPowered();
}

void* PowerControlThread(void* arg)
{
    LOG(INFO) << "Power Control thread started";
    
    while(threads_running)
    {
        ControlBatteryPower();
 
        sleep(1);
    }
    
    LOG(INFO) << "Power Control thread stopped";
    return NULL;
}

void ControlBatteryPower()
{ 
    if(IsExternalPowered() && IsJustSwitchedToExternalPower())
    {
        EnableBatteryPowered();
    }

    if(IsBatteryPowered())
    {
        if(IsLongTimeNoKeyPressed() || IsBatteryVoltageLow())
        {
            DisableBatteryPowered();
        }
    }
}

void* ChargingControlThread(void* arg)
{
    LOG(INFO) << "Charging control thread started";
    
    while(threads_running)
    {
        ControlBatteryCharging();
        sleep(5);
    }
    
    LOG(INFO) << "Charging control thread stopped";
    return NULL;
}

void ControlBatteryCharging()
{
    if(IsMinChargingVoltageReached() && !IsCharging()){
        EnableCharging();
    }

    if(IsMaxChargingVoltageReached()){
        DisableCharging();
    }
}

void* KeyMonitorThread(void* arg)
{
    LOG(INFO) << "Monitor key thread started";

    while(threads_running)
    { 
        if(lastKeyPressedTime != GetLastKeyTime())
        {
            lastKeyPressedTime = GetLastKeyTime();
        }
        sleep(1);
    }

    LOG(INFO) << "Monitor key thread stopped";
    return NULL; 
}

bool IsJustSwitchedToExternalPower()
{
    static bool batteryPoweredStatus = IsBatteryPowered();
    bool prevBatteryPoweredStatus = batteryPoweredStatus;

    batteryPoweredStatus = IsBatteryPowered();
    return prevBatteryPoweredStatus && !batteryPoweredStatus;
}


bool IsLongTimeNoKeyPressed()
{
    return (time(NULL) - lastKeyPressedTime) > keyDelayTime;
}

bool IsBatteryVoltageLow()
{
    float batteryVoltage = GetBatteryVoltage();
    return batteryVoltage < MIN_BATTERY_POWERED_VOLTAGE;
}

bool IsMinChargingVoltageReached()
{ 
    float voltage = GetBatteryVoltage();
    return voltage < MIN_CHARGING_VOLTAGE;
}

bool IsMaxChargingVoltageReached()
{
    float voltage = GetBatteryVoltage();
    return  voltage > MAX_CHARGING_VOLTAGE;
}

void EnableCharging()
{
    SetChargeStatus(1);
}

void DisableCharging()
{
    SetChargeStatus(0);
}


int GetLastKeyTime()
{
    char keyTime[14] = {0};

    DeviceFileHandler devFile("/proc/wfet/last_key_time");
    devFile.GetData( keyTime, 14 );

    LOG(INFO) << "keyTime = " << keyTime;
    return atoll( keyTime );
}

bool IsBatteryPowered()
{
    char buffer;
    DeviceFileHandler devFile("/proc/wfet/poweroff_state");
    devFile.GetData( &buffer, 1 );
    
    return ( buffer == '1' ) ? true : false;
}

bool IsExternalPowered() 
{
    return !IsBatteryPowered();
}


bool EnableBatteryPowered()
{
    char writebuff[2]={0};

    writebuff[0] = 0xF7;//掩码 开放第4位
    writebuff[1] = 0xff;//第四位写1,表示打开电池电源
    LOG(INFO) << "打开电池电源: writebuff[0]= 0xF7, writebuff[1]= 0xff";

    DeviceFileHandler devFile("/dev/wfet1000_new_gpio");
    if (!devFile.IsOpen()) return false;

    devFile.SetData(writebuff, 2);
    return true;
}

bool DisableBatteryPowered()
{
    char writebuff[2]={0};

    writebuff[0] = 0xF7;//掩码 开放第4位
    writebuff[1] = 0x00;//第四位写0,表示关闭电池电源
    LOG(INFO) << "关闭电池电源: writebuff[0]= 0xF7" << ",writebuff[1]= 0x00";

    DeviceFileHandler devFile("/dev/wfet1000_new_gpio");
    if (!devFile.IsOpen()) return false;

    devFile.SetData(writebuff, 2);
    return true;
}

float GetBatteryVoltage()
{
    DeviceFileHandler devFile("/proc/wfet/battery_adc");

    char buffer[32] = {0};
    devFile.GetData( buffer, sizeof( buffer ));

    float voltage = -1.0;
    int adcValue = 0;
    sscanf( buffer, "%x", &adcValue );

    voltage = adcValue * 9.9 / 0x3ff;
    LOG(INFO) << "Battary adcValue = " << adcValue << "voltage = " << voltage;
    return voltage;
}


bool IsCharging()
{
    DeviceFileHandler devFile( "/proc/wfet/battery_charge" );

    char buffer[16] = {0};
    devFile.GetData( buffer, sizeof( buffer ));

    int chargeStatus = 0;
    sscanf(buffer, "%d", &chargeStatus);
    LOG(INFO) << "ChargeStatus = " << chargeStatus ;

    return (chargeStatus != 0); 
}


void SetChargeStatus( int enable )
{
    char status = (enable != 0) ? '1' : '0';

    DeviceFileHandler devFile( "/proc/wfet/battery_charge" ); 
    devFile.SetData(&status, sizeof(status));
}





DeviceFileHandler::DeviceFileHandler( const char * fileName )
    : m_fileName( fileName ), m_fd(-1)
{
    m_fd = open(m_fileName.c_str(), O_RDWR);
}

DeviceFileHandler::~DeviceFileHandler()
{
    if (m_fd >= 0) {
        close(m_fd);
    }
}

bool DeviceFileHandler::Open()
{
    if (m_fd >= 0) {
        close(m_fd);
    }
    
    m_fd = open(m_fileName.c_str(), O_RDWR);
    return (m_fd >= 0);
}

bool DeviceFileHandler::IsOpen()
{
    return (m_fd >= 0);
}

int DeviceFileHandler::GetFd()
{
    return m_fd;
}   

int DeviceFileHandler::GetData( char * data, int size )
{
    if (!data || size <= 0 || m_fd < 0) {
        return -1;
    }

    ssize_t bytesRead = read(m_fd, data, size - 1);
    if (bytesRead < 0) {
        return -1;
    }
    
    data[bytesRead] = '\0';
    return bytesRead;
}

int DeviceFileHandler::SetData( const char * data, int size )
{
    if (!data || size <= 0 || m_fd < 0) {
        return -1;
    }

    ssize_t bytesWritten = write(m_fd, data, size);
    if (bytesWritten < 0) {
        return -1;
    }
    return bytesWritten;
}