#include "svc_LTU.h"
#include <math.h>

#include <pthread.h>
#include <signal.h>


pthread_t fans_control_thread;
pthread_t alarm_control_thread;


int main(int argc,char *argv[])
{
    ///485模块初始化
    Init485Module();

    if (argc > 1) 
    {
        int nSel = atoi(argv[1]);
        if(nSel == 1)
        {   
            EnableFan();
            EnableAlarmOutput();
        }
        else
        {
            DisableFan();
            DisableAlarmOutput();
        }
    }

    EnableRunningLED();
    DisableAlarmLED();   
    DisableAllLEDs();


    pthread_create(&fans_control_thread, NULL, FansControlThread, NULL);
    pthread_create(&alarm_control_thread, NULL, AlarmControlThread, NULL);  

    while (true)
    {
        LOG(DEBUG_K,"\n----------Start to read LTU data----------\n");

        for(int i = 0; i < LTU_DEVICE_NUM ; i++)
        {
            ReadAndProcessLTUData(&g_LTU_All_Volt[i]);
            ReadAndProcessLTUData(&g_LTU_All_Curr[i]);
        }
        ShowVoltageAndCurrent();

        ControlLTULEDs();
    }  
    return 0;
}

void* FansControlThread(void* arg)
{
    LOG(DEBUG_K,"\n Fans Control thread started \n");
    
    while(true)
    {
        ControlFans();
 
        sleep(10);
    }
    return NULL;
}


int ControlFans()
{
    float tempValue = GetTemperatureValue();
    
    if(tempValue > 60.0)
    {
        EnableFan();
        EnableAlarmOutput();
    }
    else if(tempValue < 50.0)
    {
        DisableFan();
        DisableAlarmOutput();
    }

    return 0;
}


void ReadAndProcessLTUData(LTU_All_Data *LTU_DataItem)
{
    ReadLTUAllData(LTU_DataItem, pmod);

    SetAllSharedMemoryData(LTU_DataItem);
}

void* AlarmControlThread(void* arg)
{
    LOG(DEBUG_K,"\n Alarm Control thread started \n");
    
    while(true)
    {
        // ControlLTULEDs();
 
        sleep(10);
    }
    return NULL;
}


void EnableRunningLED(){
    SendRemoteControlCommand(2, 0);
}

void DisableRunningLED(){
    SendRemoteControlCommand(2, 1);
}

void EnableAlarmLED(){
    SendRemoteControlCommand(1, 0);
}

void DisableAlarmLED(){
    SendRemoteControlCommand(1, 1);
}

void EnableFan(){
    SendRemoteControlCommand(3, 0);
}

void DisableFan(){
    SendRemoteControlCommand(3, 1);
}

void EnableAlarmOutput(){
    SendRemoteControlCommand(4, 0);
}

void DisableAlarmOutput(){
    SendRemoteControlCommand(4, 1);
}

void DisableAllLEDs(){
    ControlLEDGroup(0, 0);
}

float GetTemperatureValue()
{
    float tempValue = GetSharedMemoryData(SHM_TERM_DATA, 0x00510002, 0x03);
    return tempValue;
}


bool IsVoltageNormal(float Value)
{
    return (Value >= 110.0) && (Value <= 300.0);
}

void UpdateTMUVoltageLEDs()
{
    unsigned int dataIndices[] = {0x02010100, 0x02010200, 0x02010300};
    
    for (int ledIndex = 0; ledIndex < 3; ledIndex++) 
    {
        float voltValue = GetSharedMemoryData(SHM_MEASURE_DATA, dataIndices[ledIndex], 0x23);
        LOG(DEBUG_K,"TMU Phase %d Voltage: %f V\n", ledIndex + 1, voltValue);
        
        // 检查电压是否超出正常范围（110V-300V）
        if (!IsVoltageNormal(voltValue)) 
        {
            g_LedStatusIndex |= (1 << ledIndex);
        } 
        else 
        {
            g_LedStatusIndex &= ~(1 << ledIndex);
        }
    }
}

void UpdateLTUVoltageLEDs()
{
    for(int i = 0; i < LTU_DEVICE_NUM; i++)
    {
        for(int j = 0; j < LTU_PHASE_NUM; j++)
        {
            int index = LTU_PHASE_NUM * i + j;
            int sfhit = index + 3;

            float voltValue = g_LTU_All_Volt[i].value[j];

            if (!IsVoltageNormal(voltValue)) 
            {
                g_LedStatusIndex |= (1 << sfhit);
            }
            else
            {
                g_LedStatusIndex &= ~(1 << sfhit);
            }
        }
    }
}

int ControlLTULEDs()
{
    long oldLedStatusIndex = g_LedStatusIndex;

    // 检查并更新所有LED状态
    UpdateTMUVoltageLEDs();
    UpdateLTUVoltageLEDs();

    if (oldLedStatusIndex != g_LedStatusIndex) 
    {
        oldLedStatusIndex = g_LedStatusIndex;

        ControlLEDGroup(g_LedStatusIndex >> 16, g_LedStatusIndex);

        if(g_LedStatusIndex != 0)
        {
            EnableAlarmLED();
        }
        else
        {
            DisableAlarmLED();
        }
    }

    return 0;
}

void SetAlarmLED(bool alarmStatus)
{
    if(alarmStatus)
    {
        EnableAlarmLED();
    }
    else
    {
        DisableAlarmLED();
    }
}





/**
 * @brief 与LTU通信
 * @param pModuleMan 串口对象
 * @param meteraddr LTU设备地址
 * @param FrameBuf 发送数据缓冲区
 * @param iFrameLen 发送数据长度
 * @param pRecvPktBuf 接收数据缓冲区
 * @param iRecvPktBufLen 接收数据长度
 */
int CommunicateWithLTU(ModuleManEntity * pModuleMan, unsigned long long meteraddr, 
    unsigned char *FrameBuf, unsigned int iFrameLen, unsigned char *pRecvPktBuf, int &iRecvPktBufLen)
{
    if (pModuleMan == nullptr || FrameBuf == nullptr || pRecvPktBuf == nullptr) {
        LOG(ERROR_L, "Invalid parameters in CommunicateWithLTU\n");
        return -1;
    }

    LogVerboseBuffer((char *)"Send", FrameBuf, iFrameLen);

    int result = pModuleMan->MeterGather(meteraddr, (uint8_t *)FrameBuf, iFrameLen, (uint8_t *)pRecvPktBuf, iRecvPktBufLen);
    if (result != 0) {
        LOG(ERROR_L, "Failed to communicate with LTU device %llX\n", meteraddr);
        return -1;
    }

    LogVerboseBuffer((char *)"Recv", pRecvPktBuf, iRecvPktBufLen);

    return 0;
}



unsigned long long ConvertMeterAddr(unsigned long long meterAddr)
{
    char tmp[6] = {0};
    memcpy(tmp, &meterAddr, 6);  // 取6字节数据

    unsigned long long meterAddrTmp = 0;
    memcpy(&meterAddrTmp, tmp, 6);  // 转换为unsigned long long
    return meterAddrTmp;
}


/**
 * @brief 读取LTU数据
 * @param meterAddr LTU设备地址
 * @param index 数据索引
 * @param pModuleMan 串口对象
 * @return 读取到的数据，失败返回-1
 */
int ReadLTUData(unsigned long long meterAddr, int index, ModuleManEntity * pModuleMan)
{
    unsigned long long meterAddrTmp = ConvertMeterAddr(meterAddr);


    ProtocolGB645_2007 protocolGb645_07Obj;
    ProtocolGB645_2007 *pPGB456Obj = &protocolGb645_07Obj;

    int nowDataId = index;

    framesend_t stFrameSend;
    framesend_t stFrameRecv;
    unsigned char *pSendBuf =(unsigned char*)&stFrameSend;
    int sendLen = pPGB456Obj->FormReadDataFrame(pSendBuf,(unsigned char*)&meterAddrTmp,(unsigned char *)&nowDataId, NORMAL_READ,0,NULL,0);
    unsigned char *pRecvBuf = (unsigned char*)&stFrameRecv;
    int recvLen = 0;
    unsigned long long nextHopAddr =  meterAddr;

    LOG(DEBUG_K,"read645meter %llX %llX --->\n",meterAddr, nextHopAddr);

    CommunicateWithLTU(pModuleMan,nextHopAddr,pSendBuf,sendLen,pRecvBuf,recvLen);
    if(recvLen < 0)
    {
        LOG(DEBUG_K,"error,no reponse where readding 645 meter %llu\n",meterAddr);
        return -1;
    }


    char value[256] = {0};
    int valueLen = 0;

    int res = pPGB456Obj->Unpack_ReturnedFrame(pRecvBuf,recvLen,(unsigned char*)value,valueLen);
    if(res != 0)
    {
        LOG(DEBUG_K,"parse get response of 645 error\n");

        return -1;
    }
    LogVerboseBuffer((char*)"receive value:",(unsigned char*)value,valueLen);

    int result = (((value[2] & 0x70) >> 4) * 100000) + ((value[2] & 0x0F) * 10000) + \
                 (((value[1] & 0xF0) >> 4) * 1000) + ((value[1] & 0x0F) * 100) + \
                 (((value[0] & 0xF0) >> 4) * 10) + (value[0] & 0x0F)  ;

    if(value[2] & 0x80)
    {
        result = -result;
    }

    return result;
}


int ReadLTUAllData(LTU_All_Data *LTU_DataItem, ModuleManEntity * pModuleMan)
{
    unsigned long long meterAddrTmp = 0;
    char tmp[6] = {0};
    memcpy(tmp,&LTU_DataItem->deviceAddr,6);
    memcpy(&meterAddrTmp,tmp,6);

    ProtocolGB645_2007 protocolGb645_07Obj;
    ProtocolGB645_2007 *pPGB456Obj = &protocolGb645_07Obj;

    int nowDataId = LTU_DataItem->dataIndex;

    framesend_t stFrameSend;
    framesend_t stFrameRecv;
    unsigned char *pSendBuf =(unsigned char*)&stFrameSend;
    int sendLen = pPGB456Obj->FormReadDataFrame(pSendBuf,(unsigned char*)&meterAddrTmp,(unsigned char *)&nowDataId, NORMAL_READ,0,NULL,0);
    unsigned char *pRecvBuf = (unsigned char*)&stFrameRecv;
    int recvLen = 0;
    unsigned long long nextHopAddr =  LTU_DataItem->deviceAddr;

    
    LOG(DEBUG_K,"read645meter %llX %llX --->\n",LTU_DataItem->deviceAddr, nextHopAddr);

    CommunicateWithLTU(pModuleMan,nextHopAddr,pSendBuf,sendLen,pRecvBuf,recvLen);

    if(recvLen < 0)
    {
        LOG(DEBUG_K,"error,no reponse where readding 645 meter %llu\n",LTU_DataItem->deviceAddr);
        return -1;
    }





    int valueLen = 0;
    int res = pPGB456Obj->Unpack_ReturnedFrame(pRecvBuf,recvLen,(unsigned char*)LTU_DataItem->receiveBuf,valueLen);
    if(res != 0)
    {
        LOG(DEBUG_K,"parse get response of 645 error\n");
        return -1;
    }
    LogVerboseBuffer((char*)"receive value:",(unsigned char*)LTU_DataItem->receiveBuf,valueLen);






    int dateByte = LTU_DataItem->dateByte;

    // 使用临时缓冲区处理数据，避免直接修改receiveBuf
    unsigned char tempBuf[MAX_RECV_LEN] = {0};
    memcpy(tempBuf, LTU_DataItem->receiveBuf, sizeof(tempBuf));

    for (int i = 0; i < LTU_DataItem->dateNum; i++)
    {
        int result = 0;
        int negativeFlag = 0;

        // 处理负数标志
        if (LTU_DataItem->isNegative)
        {
            negativeFlag = tempBuf[i * dateByte + dateByte - 1] & 0x80;
            tempBuf[i * dateByte + dateByte - 1] &= 0x7F;
        }

        // 解析BCD编码数据
        for (int j = 1; j <= dateByte ; j++)
        {
            unsigned char byte = tempBuf[(i + 1) * dateByte - j];
            unsigned char high_nibble = (byte & 0xF0) >> 4;
            unsigned char low_nibble = byte & 0x0F;
            result = (result *100) + (high_nibble * 10) + low_nibble;
        }
        
        // 应用符号
        if(negativeFlag)
        {
            result = -result;
        }
        
        // 缩放并过滤小值
        float tempvalue = (float)result/(LTU_DataItem->scale);
        LTU_DataItem->value[i] = (fabs(tempvalue) > 0.05) ? tempvalue : 0.0;
        
        LOG(DEBUG_K,"LTU_DataItem->value[%d] = %f\n",i,LTU_DataItem->value[i]);
    }

    return 0;
}



/**
 * @ Description:CRC校验
 */
static unsigned short CalcCrc16_B( int recv_len, unsigned char *recv, int offset)
{
    unsigned short crc_temp, i, j, flag;
    if ( recv_len - offset <= 0 )
    {
        return 0;
    }
    crc_temp = 0xFFFF;
    for( i=0; i<recv_len; i++)
    {
        crc_temp ^= ( recv[offset+i]&0xff );
        for ( j=0; j<8; j++)
        {
            flag = crc_temp&0x0001;		//获取即将右移出去的位值
            crc_temp >>= 1;
            if ( flag )
            {
                crc_temp ^= 0xA001;
            }
        }
    }
    return crc_temp;
}

/**
 * @brief 组modbus数据帧
 * 
 * @param Addr  
 * @param buff
 * @param func 
 * @param dataitem 
 * @param itemlen 
 * @return int 
 */
static int CreateReadDataFrame(char Addr, unsigned char *buff, char func, long dataitem, long itemlen)
{
    short   crc16;
    buff[0] = Addr;
    buff[1] = func;
    buff[2] = dataitem >> 8;
    buff[3] = dataitem;
    buff[4] = itemlen >> 8;
    buff[5] = itemlen;
    crc16 = CalcCrc16_B( 6, buff, 0 );
    buff[6] = crc16&0xff;
    buff[7] = (crc16>>8)&0xff;
    return  8;
}

// 校验和
static int GetCheckSum(const uint8_t *buff, int buffLen)
{
    int init = 0x00;
    for (int i = 0; i< buffLen; i ++)  	
    {
        init += *(buff+i);
    }					

    return (init & 0xFF);
}


static int CreateControlFrame(uint8_t* buffer, int portNum,  int state)
{
    if (buffer == NULL) 
    {
        printf("input parameter is invalid.\n");
        return -1;  // 指针无效，返回错误
    }

    int apsub = 0; // 索引变量

    // 帧头
    buffer[apsub++] = 0x68;
    buffer[apsub++] = 0x28;
    buffer[apsub++] = 0x03;

    // 数据域
    buffer[apsub++] = portNum;
    buffer[apsub++] = 0x63;
    buffer[apsub++] = state;
	
    // 帧尾
    int checksum = GetCheckSum(buffer, apsub);
    buffer[apsub++] = checksum;
    buffer[apsub++] = 0x16;

    return apsub;
}

int ControlLEDGroup(int index, int status)
{
    unsigned char w_buf[10]={0x0};
    unsigned char r_buf[MAX_RECV_LEN]={0x0};
    int recvLen = 10;
    int sendLen = 0;

    int frameLen = CreateReadDataFrame(0x01, w_buf, 0x07, index, status);
    if (frameLen <= 0) {
        LOG(ERROR_L, "Failed to create LED group control frame for index %d, status %d\n", index, status);
        return -1;
    }

    LogVerboseBuffer((char *)"Send", w_buf, 10);

    int result = pmod2->MeterGather(0x00, (uint8_t *)w_buf, recvLen, (uint8_t *)r_buf, sendLen);
    if (result != 0) {
        LOG(ERROR_L, "Failed to control LED group for index %d, status %d\n", index, status);
        return -1;
    }

    LogVerboseBuffer((char *)"Recv", r_buf, sendLen);
    
    return 0;
}


int SendRemoteControlCommand(int portNum, int state)
{
	unsigned char w_buf[10]={0x0};
    unsigned char r_buf[MAX_RECV_LEN]={0x0};
    int recvLen = 10;
    int sendLen = 0;

    int frameLen = CreateControlFrame((uint8_t*)w_buf, portNum, state);
    if (frameLen <= 0) {
        LOG(ERROR_L, "Failed to create control frame for port %d, state %d\n", portNum, state);
        return -1;
    }

    LogVerboseBuffer((char *)"Send", w_buf, 10);

    int result = pmod2->MeterGather(0x00, (uint8_t *)w_buf, recvLen, (uint8_t *)r_buf, sendLen);
    if (result != 0) {
        LOG(ERROR_L, "Failed to send remote control command for port %d, state %d\n", portNum, state);
        return -1;
    }

    LogVerboseBuffer((char *)"Recv", r_buf, sendLen);
    
    return 0;
}










void Init485Module()
{
    // 485端口-LTU初始化-COM4
    if(NULL==pmod)
    {
        pmod = Create485Module(COM4);
        if(NULL==pmod)
        {
            LOG(ERROR_L,"%s,%d,Create 485_%d Module Fail\n",__FILE__,__LINE__,COM4);
            return;
        }
    }

    /// 485模块-显控板初始化-COM6
    if(NULL==pmod2)
    {
        pmod2 = Create485Module(COM6);
        if(NULL==pmod2)
        {
            LOG(ERROR_L,"%s,%d,Create 485_%d Module Fail\n",__FILE__,__LINE__,COM4);
            return;
        }
    }
}


/**
 * @brief 创建485模块对象
 * @return ModuleManEntity* 485模块对象指针
 */
ModuleManEntity* Create485Module(int comNum)
{
    S_COMM_PARA commParam;
    memset(&commParam,0,sizeof(commParam));
    commParam.baudrate   = 9600;
    commParam.parity     = PARITY_ODD;          //校验
    commParam.sync_mode  = 0;                   //同步方式
    commParam.rtscts     = COM_RTSCTS_DISABLE;  //流控制
    commParam.bytesize   = COM_BYTESIZE8;       //数据位长度
    commParam.stopbits   = 1;                   //停止位长度
    commParam.serial_num = comNum;
    CVirtualSerial *pSerial = new CSerialComm(&commParam);
    if(NULL==pSerial)
    {
        LOG(ERROR_L,"%S,%d,Serial Port Initialization Failed.\n",__FILE__,__LINE__);
        return NULL;
    }
    ModuleManEntity *pModule = new Relay(pSerial);
    if(NULL==pModule)
    {
        LOG(ERROR_L,"%S,%d,Relay Module Initialization Failed.\n",__FILE__,__LINE__);
        return NULL;
    }
    pModule->Init((const vector<struct MeterDoc>)0, 0);

    return pModule;
}



int SetSharedMemoryData(LTU_Data *LTU_DataItem)
{
    //传感器数据需要写内存，内存初始化
    CShmIO shm;
    shm.Init(SHM_TERM_DATA,0,CREATE);

    int len = 4;
    shm.SetItem(&LTU_DataItem->value, len, LTU_DataItem->shmAddr, LTU_DataItem->shmSubAddr);

    return 0;
}

int SetAllSharedMemoryData(LTU_All_Data *LTU_DataItem)
{
    //传感器数据需要写内存，内存初始化
    CShmIO shm;
    shm.Init(SHM_TERM_DATA,0,CREATE);

    int len = 4;
    for(int i = 0; i < LTU_DataItem->dateNum; i++)
    {
        shm.SetItem(&LTU_DataItem->value[i], len, LTU_DataItem->shmAddr, 0x04 + i);
    }

    return 0;
}

float GetSharedMemoryData(int shmLibrary, unsigned int shmAddr, unsigned int shmSubAddr)
{
    int len = 0;
    float value = 0.0;
    unsigned char dataBuf[128] = {0};

    //传感器数据需要写内存，内存初始化
    CShmIO shm;
    shm.Init(shmLibrary,0,RWRITE);

    shm.GetItem(&dataBuf, len, shmAddr, shmSubAddr);

    memcpy(&value,dataBuf,4);
    printf("outData =%0.3f\n",value);

    return value;
}

/**
 * @brief  打印数据包内容
 * 
 * @param szDirect 
 * @param pBuf 
 * @param usLen 
 */
void LogVerboseBuffer(const char *szDirect, unsigned char *pBuf, unsigned short usLen)
{
    char cSep;
    char *szBuf = (char *)malloc(usLen * 3 + 100); /* 内容 余者 */
    if (NULL == szBuf)
    {
        return;
    }
    
    char *pTmp = szBuf;
    for (int i=0; i < usLen; i++)
    {
        cSep = (i % 16 == 15) ? '\n' : ' ';
        pTmp += sprintf(pTmp, "%02X%c", pBuf[i], cSep);
    }
    pTmp += sprintf(pTmp, "\n");
    LOG(DEBUG_K,"\n%s[%d] : %s\n", szDirect, usLen, szBuf);

    free(szBuf);
    return;
}


void ShowVoltageAndCurrent()
{
    //打印电压电流值
    LOG(DEBUG_K,"LTU Voltage And Current Value:");

    for(int i = 0; i < LTU_DEVICE_NUM; i++)
    {
         LOG(DEBUG_K,"\n%f/%f      %f/%f      %f/%f",\
                    g_LTU_All_Volt[i].value[0],g_LTU_All_Curr[i].value[0],\
                    g_LTU_All_Volt[i].value[1],g_LTU_All_Curr[i].value[1],\
                    g_LTU_All_Volt[i].value[2],g_LTU_All_Curr[i].value[2]);

    }
     LOG(DEBUG_K,"\n");
    
    return;
}