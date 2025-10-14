
#ifndef _STACK3762_H_
#define _STACK3762_H_

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

//帧检查错误码定义
typedef enum GDW3762_FRMAE_CHECK_ERR_CODE
{
    GDW3762_FRMAE_CHECK_SUCCESS=0,
    GDW3762_FRAME_CHECK_ERROR=1,
    GDW3762_FRAME_CHECK_CRC_ERROR=2,
    GDW3762_FRAME_CHECK_NOT_COMPLETE=3,
}GDW3762_FRMAE_CHECK_ERR_CODE_E;

#define GDW3762_MAX_FRAME_LEN 1080 /* 模块支持数据长度为1024，加头部并加点余量 */
#define GDW3762_FRAME_START_FLAG 0X68
#define GDW3762_FRAME_TAIL_FLAG 0X16
#define GDW3762_MAC_ADDR_LEN  6
#define GDW3762_MAX_RELAY_CNT  15
/******************************************************************
                       下面是控制域相关定义
******************************************************************/
typedef enum
{
    GDW3762_DIR_DOWN = 0,  /* 下行 */
    GDW3762_DIR_UP = 1,    /* 上行 */
}GDW3762_DIR_E;

typedef enum
{
    GDW3762_PRM_DRIVEN = 0,  /* 从动 */
    GDW3762_PRM_STARTUP = 1, /* 主动 */
}GDW3762_PRM_E;

typedef enum
{
    GDW3762_COMM_MODE_CENTRALIZED_ROUTING_CARRIER = 1,  /* 集中式路由载波 */
    GDW3762_COMM_MODE_DISTRIBUTE_ROUTING_CARRIER = 2,   /* 分布式路由载波 */
    GDW3762_COMM_MODE_LOW_POWER_RF = 10,   /* 分布式路由载波 */
    GDW3762_COMM_MODE_ETHERNET = 20,   /* 分布式路由载波 */
}GDW3762_COMM_MODE_E;

typedef union uGDW3762_CONTROL_FORMAT
{
	unsigned char Val;
	struct
	{
        unsigned char CommMode : 6;
        unsigned char Prm : 1;
		unsigned char Dir: 1;
	}__attribute__((packed))bits;
} __attribute__((packed)) GDW3762_CONTROL_FORMAT_U;

/* 帧头帧尾格式，中间夹用户数据区 */
typedef struct tagGDW3762_FRAME_HEAD
{
	unsigned char HeadFlag;
    unsigned short Len;
    GDW3762_CONTROL_FORMAT_U Control;
} __attribute__((packed)) GDW3762_FRAME_HEAD_S;

typedef struct tagGDW3762_FRAME_TAIL
{
	unsigned char FCS;
	unsigned char TailFlag;
} __attribute__((packed)) GDW3762_FRAME_TAIL_S;

/* 下行方向通信参数 */
typedef union uGDW3762_DOWNLINK_COMM_PARAM
{
	unsigned short Val;
	struct
	{
		unsigned short RouteFlag: 1;  /* 0表示通信模块带路由或工作在路由模式，1表示通信模块不带路由或工作在旁路模式 */
        unsigned short AppendNodeFlag : 1; /*  指载波从节点附属节点标识，0表示无附加节点，1表示有附加节点  */
        unsigned short CommModuleFlag : 1; /*  0表示对集中器的通信模块操作，1表示对载波表的通信模块操作  */
        unsigned short ConflictDetectFlag : 1; /* 0表示不进行冲突检测，1表示要进行冲突检测 */
        unsigned short RelayLevel : 4; /* 取值范围0~15，0表示无中继 */
        unsigned short ChannelType : 4; /* 取值0~15，0表示不分信道、1~15依次表示第1~15信道 */
        unsigned short ECCType : 4; /* 取值范围0~15，0表示信道未编码，1表示RS编码，2~15保留 */
	}__attribute__((packed))bits;
} __attribute__((packed)) GDW3762_DOWNLINK_COMM_PARAM_U;

/* 下行方向的信息域结构 */
typedef struct tagGDW3762_DOWN_INFOR
{
	GDW3762_DOWNLINK_COMM_PARAM_U LinkParam;
    unsigned char ReplyByteNum; /* 取值0~255，用于计算延时等待时间；为0时，延时等待时间为默认时间 */
    unsigned short CommSpeedValue : 15; /* 表示通信波特率，BIN格式，0表示默认通信速率 */
    unsigned short CommSpeedUnit : 1; /* 0表示bps，1表示kbps */
    unsigned char seqNum; /* 预留字节,在1376.2改为帧序列号 */
} __attribute__((packed)) GDW3762_DOWN_INFOR_S;

/* 上行方向的通信参数 */
typedef union uGDW3762_UPLINK_COMM_PARAM
{
	unsigned short Val;
	struct
	{
		unsigned short RouteFlag: 1;  /* 0表示通信模块带路由或工作在路由模式，1表示通信模块不带路由或工作在旁路模式 */
        unsigned short resv1 : 1; 
        unsigned short CommModuleFlag : 1; /*  0表示对集中器的通信模块操作，1表示对载波表的通信模块操作  */
        unsigned short resv2 : 1; 
        unsigned short RelayLevel : 4; /* 路由级别，取值范围0~15，0表示无中继 */
        unsigned short ChannelType : 4; /* 取值0~15，0表示不分信道、1~15依次表示第1~15信道 */
        unsigned short resv3 : 4; 
	}__attribute__((packed))bits;
} __attribute__((packed)) GDW3762_UPLINK_COMM_PARAM_U;

/* 上行方向的表计参数 */
typedef union uGDW3762_UPLINK_METER_PARAM
{
	unsigned short Val;
	struct
	{
		unsigned short Phase: 4;  /* 实测从节点逻辑主信道所在电源相别，0为不确定，1~3依次表示相别为第1相、第2相、第3相 */
        unsigned short MeterChannelCharacter : 4; /* 描述目的节点电表通道的特征，取值范围0~15，0保留，
        1为载波物理信道为单相供电，逻辑信道为单信道；2为载波物理信道为单相供电，逻辑信道为两信道；
        3为载波物理信道为单相供电，逻辑信道为三信道；4为载波物理信道为三相供电，逻辑信道为三信道 */
        unsigned short TailCommandLQI : 4; /*  分为15级，取值范围0~15，0表示无信号品质，1表示最低品质  */
        unsigned short TailResponseLQI : 4; /*  分为15级，取值范围0~15，0表示无信号品质，1表示最低品质  */
	}__attribute__((packed))bits;
} __attribute__((packed)) GDW3762_UPLINK_METER_PARAM_U;

/* 上行方向的表计参数 */
typedef union uGDW3762_RSV_AND_SEQ
{
	unsigned short Val;
	struct
	{
		unsigned short Resv: 7;  
        unsigned short EventFalg : 1; 
        unsigned short SeqNum : 8; 
	}__attribute__((packed))bits;
} __attribute__((packed)) GDW3762_UPLINK_RSV_AND_SEQ_U;


typedef struct tagGDW3762_UP_INFOR
{
	GDW3762_UPLINK_COMM_PARAM_U LinkParam;
	GDW3762_UPLINK_METER_PARAM_U MeterParam;
    GDW3762_UPLINK_RSV_AND_SEQ_U Resv;
} __attribute__((packed)) GDW3762_UP_INFOR_S;

typedef struct tagGDW3762_DT
{
	unsigned char dt1;
	unsigned char dt2;
}__attribute__((packed))GDW3762_DT;

typedef unsigned char GDW3762_ADDR_S[GDW3762_MAC_ADDR_LEN] ;

typedef struct//数据域
{
	unsigned short dataLen;
	unsigned char data[GDW3762_MAX_FRAME_LEN];
}__attribute__((packed))GDW3762_DATA;

typedef struct tagGDW3762_FARME_INFO
{
	GDW3762_CONTROL_FORMAT_U Control;
	GDW3762_DOWN_INFOR_S DownInfo;
	GDW3762_UP_INFOR_S   UpInfo;
	GDW3762_ADDR_S DestAddr;
	GDW3762_ADDR_S SrcAddr;
    GDW3762_ADDR_S relayAddr[GDW3762_MAX_RELAY_CNT];
	unsigned char afn;
    unsigned char fn;
	GDW3762_DT dt; // Fn
	GDW3762_DATA dataUnit;
}__attribute__((packed))GDW3762_FARME_INFO_S;




/* 具体业务报文的数据部分格式和长度定义 */
#define GDW3762_AFN00FN1_DATALEN 4
#define GDW3762_AFN00FN2_DATALEN 1
typedef enum
{
    GDW3762_NAK_TIMEOUT = 0,  /* 通信超时 */
    GDW3762_NAK_INVALID_DATA_UNIT = 1,    /* 无效数据单元 */
    GDW3762_NAK_INVALID_LEN = 2,    /* 长度错 */
    GDW3762_NAK_INVALID_FCS = 3,    /* 校验错误 */
    GDW3762_NAK_INFOR_CLASS_NOT_EXIST = 4,    /* 信息类不存在 */
    GDW3762_NAK_INVALID_FORMAT = 5,    /* 格式错误 */
    GDW3762_NAK_METER_ADDR_DUPLICATE = 6,    /* 表号重复 */
    GDW3762_NAK_METER_ADDR_NOT_EXIST = 7,    /* 表号重复 */
    GDW3762_NAK_METER_APP_NO_RESPONSE = 8,    /* 电表应用层无应答 */
}GDW3762_NAK_ERROR_CODE_E;

typedef struct tagGDW3762_AFN10F2_DOWN_DATA
{
    unsigned short startNo;
    unsigned char number;
}__attribute__((packed))GDW3762_AFN10F2_DOWN_DATA_S;

typedef enum
{
    GDW3762_METER_PROTOCOL_TRANSPARENT = 0,
    GDW3762_METER_PROTOCOL_64597 = 1,
    GDW3762_METER_PROTOCOL_64507 = 2,
}GDW3762_METER_PROTOCOL_E;

typedef struct tagGDW3762_AFN11F1_SLAVE_NODE_INFO
{
    GDW3762_ADDR_S nodeAddr;
    //unsigned short NodeNo;
    unsigned char MeterPro; /* GDW3762_METER_PROTOCOL_E */
}__attribute__((packed))GDW3762_AFN11F1_SLAVE_NODE_INFO_S;

typedef struct tagGDW3762_DATE_TIME
{
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned char year;
}__attribute__((packed))GDW3762_DATE_TIME_S;

typedef struct tagGDW3762_AFN11F5_SLAVE_AUTO_REG
{
    GDW3762_DATE_TIME_S startTime;
    unsigned short durationMinute;
    unsigned char retryTimes;
    unsigned char randomWaitSlice; /* 随机等待时间片个数：时间片指150ms */
}__attribute__((packed))GDW3762_AFN11F5_SLAVE_AUTO_REG_S;

typedef struct tagGDW3762_AFN15F1_TRANS_FILE
{
    unsigned char fileFlag;         //文件标识
    unsigned char fileAttr;         //文件属性
    unsigned char fileCmd;          //文件指令
    unsigned short fileTotalPack;   //文件总段数
    unsigned long filePackFlag;     //文件段标识
    unsigned short filePackLen;     //文件段长度
    unsigned char *filePackData;        //文件段数据体
}__attribute__((packed))GDW3762_AFN15F1_TRANS_FILE_S;

typedef struct tagGDW3762_AFN05F5_PARAM
{
    unsigned char wiChinnel;        //通信信道
    unsigned char wiPower;         //无线发射功率
}__attribute__((packed))GDW3762_AFN05F5_PARAM_S;



class cGDW3762_FARME
{
public:
	cGDW3762_FARME(void);
	~cGDW3762_FARME(void);
	int FormSendFrame();
	int ParseRecvFrame();
	int AdParseRecvFrame(uint8_t *pInBuf,int bufLen);
    void SetSendFrameDefaultParam();

	int FormAfn00F1(GDW3762_PRM_E prm, unsigned char *data, unsigned char dataLen,unsigned char seqnum); // data为NULL则用默认值填充
	int FormAfn00F2(GDW3762_PRM_E prm, GDW3762_NAK_ERROR_CODE_E errorCode);
	
    int FormAfnF0F23(unsigned char *pInBuff,int inBuffLen);
    int FormAfnF0F21(void);
    int FormAfnF0F22(unsigned short startNodeNo, unsigned char nodeNum);
    
    int FormAfn01(unsigned char fn);
    int FormAfn02F1(GDW3762_METER_PROTOCOL_E proTye, unsigned short dataLen, unsigned char *data,
                                GDW3762_ADDR_S destAddr, GDW3762_ADDR_S srcAddr);
    int FormAfn02F2(GDW3762_METER_PROTOCOL_E proTye, unsigned short dataLen, unsigned char *data,
                                GDW3762_ADDR_S destAddr, GDW3762_ADDR_S srcAddr);
    int FormAfn03(unsigned char fn);
    int FormAfn03F21(GDW3762_ADDR_S addr);
    int FormAfn03F100(unsigned char * mBuf,unsigned short bufLen);
	int FormAfn03F102 ();
    int FormAfn04F2(GDW3762_ADDR_S srcAddr,GDW3762_ADDR_S addr);
	int FormAfn04F3(GDW3762_ADDR_S srcAddr,GDW3762_ADDR_S addr,unsigned char cnt);
    int FormAfn05F1(GDW3762_ADDR_S addr);
	int FormAfn05F4(int mValue);

	int FormAfn05F5(GDW3762_AFN05F5_PARAM_S mParam);
    int FormAfn05F100(unsigned char * mBuf,unsigned short bufLen);
    int FormAfn10F1();
    int FormAfn10F2(GDW3762_AFN10F2_DOWN_DATA_S *data);
    int FormAfn10F21(short nodeStartIdx,unsigned char nodeNum);
    int FormAfn11F1(vector <GDW3762_AFN11F1_SLAVE_NODE_INFO_S> slaveNode);
    int FormAfn11F2(vector <GDW3762_ADDR_S> slaveNode);
    int FormAfn11F5(GDW3762_AFN11F5_SLAVE_AUTO_REG_S *regInfo);
    int FormAfn11F6(void);
    int FormAfn12(unsigned char fn);
    int FormAfn13F1(GDW3762_METER_PROTOCOL_E meterPro, vector <GDW3762_ADDR_S> meterList,
        unsigned char *pData, unsigned short dataLen);
    int FormAfn13F2(GDW3762_METER_PROTOCOL_E meterPro, vector <GDW3762_ADDR_S> meterList,
        unsigned char *pData, unsigned short dataLen);
    
    int FormAfn15F1(tagGDW3762_AFN15F1_TRANS_FILE *transFileSt,GDW3762_ADDR_S destAddr, GDW3762_ADDR_S srcAddr);
	int FormAfn05F3(GDW3762_METER_PROTOCOL_E proTye,unsigned short dataLen, unsigned char *data);
	GDW3762_FARME_INFO_S   gdwSendFarmeInfo; //376.2发送帧结构
	GDW3762_FARME_INFO_S   gdwRecvFarmeInfo;   //376.2接收帧结构

    GDW3762_DATA   sendFarme; //发送帧报文
    GDW3762_DATA   recvFarme; //经帧检查后获得的接收帧报文
    static unsigned char sendSeq;
	bool SendFrameIsAfn03F1;
};
unsigned char CalcFcs(unsigned char *ptr,unsigned short len);
int GDW3762_EncodeFn(unsigned char fn, unsigned char &dt1, unsigned char &dt2);
int GDW3762_DecodeFn(unsigned char dt1, unsigned char dt2, unsigned char &fn);
int GDW3762_CheckRecvFrame(unsigned char *recvbuf, int recvbuflen, int &offset, int &frameLen);

#endif

