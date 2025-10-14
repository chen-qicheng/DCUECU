
#ifndef _STACK3762_H_
#define _STACK3762_H_

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

//֡�������붨��
typedef enum GDW3762_FRMAE_CHECK_ERR_CODE
{
    GDW3762_FRMAE_CHECK_SUCCESS=0,
    GDW3762_FRAME_CHECK_ERROR=1,
    GDW3762_FRAME_CHECK_CRC_ERROR=2,
    GDW3762_FRAME_CHECK_NOT_COMPLETE=3,
}GDW3762_FRMAE_CHECK_ERR_CODE_E;

#define GDW3762_MAX_FRAME_LEN 1080 /* ģ��֧�����ݳ���Ϊ1024����ͷ�����ӵ����� */
#define GDW3762_FRAME_START_FLAG 0X68
#define GDW3762_FRAME_TAIL_FLAG 0X16
#define GDW3762_MAC_ADDR_LEN  6
#define GDW3762_MAX_RELAY_CNT  15
/******************************************************************
                       �����ǿ�������ض���
******************************************************************/
typedef enum
{
    GDW3762_DIR_DOWN = 0,  /* ���� */
    GDW3762_DIR_UP = 1,    /* ���� */
}GDW3762_DIR_E;

typedef enum
{
    GDW3762_PRM_DRIVEN = 0,  /* �Ӷ� */
    GDW3762_PRM_STARTUP = 1, /* ���� */
}GDW3762_PRM_E;

typedef enum
{
    GDW3762_COMM_MODE_CENTRALIZED_ROUTING_CARRIER = 1,  /* ����ʽ·���ز� */
    GDW3762_COMM_MODE_DISTRIBUTE_ROUTING_CARRIER = 2,   /* �ֲ�ʽ·���ز� */
    GDW3762_COMM_MODE_LOW_POWER_RF = 10,   /* �ֲ�ʽ·���ز� */
    GDW3762_COMM_MODE_ETHERNET = 20,   /* �ֲ�ʽ·���ز� */
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

/* ֡ͷ֡β��ʽ���м���û������� */
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

/* ���з���ͨ�Ų��� */
typedef union uGDW3762_DOWNLINK_COMM_PARAM
{
	unsigned short Val;
	struct
	{
		unsigned short RouteFlag: 1;  /* 0��ʾͨ��ģ���·�ɻ�����·��ģʽ��1��ʾͨ��ģ�鲻��·�ɻ�������·ģʽ */
        unsigned short AppendNodeFlag : 1; /*  ָ�ز��ӽڵ㸽���ڵ��ʶ��0��ʾ�޸��ӽڵ㣬1��ʾ�и��ӽڵ�  */
        unsigned short CommModuleFlag : 1; /*  0��ʾ�Լ�������ͨ��ģ�������1��ʾ���ز����ͨ��ģ�����  */
        unsigned short ConflictDetectFlag : 1; /* 0��ʾ�����г�ͻ��⣬1��ʾҪ���г�ͻ��� */
        unsigned short RelayLevel : 4; /* ȡֵ��Χ0~15��0��ʾ���м� */
        unsigned short ChannelType : 4; /* ȡֵ0~15��0��ʾ�����ŵ���1~15���α�ʾ��1~15�ŵ� */
        unsigned short ECCType : 4; /* ȡֵ��Χ0~15��0��ʾ�ŵ�δ���룬1��ʾRS���룬2~15���� */
	}__attribute__((packed))bits;
} __attribute__((packed)) GDW3762_DOWNLINK_COMM_PARAM_U;

/* ���з������Ϣ��ṹ */
typedef struct tagGDW3762_DOWN_INFOR
{
	GDW3762_DOWNLINK_COMM_PARAM_U LinkParam;
    unsigned char ReplyByteNum; /* ȡֵ0~255�����ڼ�����ʱ�ȴ�ʱ�䣻Ϊ0ʱ����ʱ�ȴ�ʱ��ΪĬ��ʱ�� */
    unsigned short CommSpeedValue : 15; /* ��ʾͨ�Ų����ʣ�BIN��ʽ��0��ʾĬ��ͨ������ */
    unsigned short CommSpeedUnit : 1; /* 0��ʾbps��1��ʾkbps */
    unsigned char seqNum; /* Ԥ���ֽ�,��1376.2��Ϊ֡���к� */
} __attribute__((packed)) GDW3762_DOWN_INFOR_S;

/* ���з����ͨ�Ų��� */
typedef union uGDW3762_UPLINK_COMM_PARAM
{
	unsigned short Val;
	struct
	{
		unsigned short RouteFlag: 1;  /* 0��ʾͨ��ģ���·�ɻ�����·��ģʽ��1��ʾͨ��ģ�鲻��·�ɻ�������·ģʽ */
        unsigned short resv1 : 1; 
        unsigned short CommModuleFlag : 1; /*  0��ʾ�Լ�������ͨ��ģ�������1��ʾ���ز����ͨ��ģ�����  */
        unsigned short resv2 : 1; 
        unsigned short RelayLevel : 4; /* ·�ɼ���ȡֵ��Χ0~15��0��ʾ���м� */
        unsigned short ChannelType : 4; /* ȡֵ0~15��0��ʾ�����ŵ���1~15���α�ʾ��1~15�ŵ� */
        unsigned short resv3 : 4; 
	}__attribute__((packed))bits;
} __attribute__((packed)) GDW3762_UPLINK_COMM_PARAM_U;

/* ���з���ı�Ʋ��� */
typedef union uGDW3762_UPLINK_METER_PARAM
{
	unsigned short Val;
	struct
	{
		unsigned short Phase: 4;  /* ʵ��ӽڵ��߼����ŵ����ڵ�Դ���0Ϊ��ȷ����1~3���α�ʾ���Ϊ��1�ࡢ��2�ࡢ��3�� */
        unsigned short MeterChannelCharacter : 4; /* ����Ŀ�Ľڵ���ͨ����������ȡֵ��Χ0~15��0������
        1Ϊ�ز������ŵ�Ϊ���๩�磬�߼��ŵ�Ϊ���ŵ���2Ϊ�ز������ŵ�Ϊ���๩�磬�߼��ŵ�Ϊ���ŵ���
        3Ϊ�ز������ŵ�Ϊ���๩�磬�߼��ŵ�Ϊ���ŵ���4Ϊ�ز������ŵ�Ϊ���๩�磬�߼��ŵ�Ϊ���ŵ� */
        unsigned short TailCommandLQI : 4; /*  ��Ϊ15����ȡֵ��Χ0~15��0��ʾ���ź�Ʒ�ʣ�1��ʾ���Ʒ��  */
        unsigned short TailResponseLQI : 4; /*  ��Ϊ15����ȡֵ��Χ0~15��0��ʾ���ź�Ʒ�ʣ�1��ʾ���Ʒ��  */
	}__attribute__((packed))bits;
} __attribute__((packed)) GDW3762_UPLINK_METER_PARAM_U;

/* ���з���ı�Ʋ��� */
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

typedef struct//������
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




/* ����ҵ���ĵ����ݲ��ָ�ʽ�ͳ��ȶ��� */
#define GDW3762_AFN00FN1_DATALEN 4
#define GDW3762_AFN00FN2_DATALEN 1
typedef enum
{
    GDW3762_NAK_TIMEOUT = 0,  /* ͨ�ų�ʱ */
    GDW3762_NAK_INVALID_DATA_UNIT = 1,    /* ��Ч���ݵ�Ԫ */
    GDW3762_NAK_INVALID_LEN = 2,    /* ���ȴ� */
    GDW3762_NAK_INVALID_FCS = 3,    /* У����� */
    GDW3762_NAK_INFOR_CLASS_NOT_EXIST = 4,    /* ��Ϣ�಻���� */
    GDW3762_NAK_INVALID_FORMAT = 5,    /* ��ʽ���� */
    GDW3762_NAK_METER_ADDR_DUPLICATE = 6,    /* ����ظ� */
    GDW3762_NAK_METER_ADDR_NOT_EXIST = 7,    /* ����ظ� */
    GDW3762_NAK_METER_APP_NO_RESPONSE = 8,    /* ���Ӧ�ò���Ӧ�� */
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
    unsigned char randomWaitSlice; /* ����ȴ�ʱ��Ƭ������ʱ��Ƭָ150ms */
}__attribute__((packed))GDW3762_AFN11F5_SLAVE_AUTO_REG_S;

typedef struct tagGDW3762_AFN15F1_TRANS_FILE
{
    unsigned char fileFlag;         //�ļ���ʶ
    unsigned char fileAttr;         //�ļ�����
    unsigned char fileCmd;          //�ļ�ָ��
    unsigned short fileTotalPack;   //�ļ��ܶ���
    unsigned long filePackFlag;     //�ļ��α�ʶ
    unsigned short filePackLen;     //�ļ��γ���
    unsigned char *filePackData;        //�ļ���������
}__attribute__((packed))GDW3762_AFN15F1_TRANS_FILE_S;

typedef struct tagGDW3762_AFN05F5_PARAM
{
    unsigned char wiChinnel;        //ͨ���ŵ�
    unsigned char wiPower;         //���߷��书��
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

	int FormAfn00F1(GDW3762_PRM_E prm, unsigned char *data, unsigned char dataLen,unsigned char seqnum); // dataΪNULL����Ĭ��ֵ���
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
	GDW3762_FARME_INFO_S   gdwSendFarmeInfo; //376.2����֡�ṹ
	GDW3762_FARME_INFO_S   gdwRecvFarmeInfo;   //376.2����֡�ṹ

    GDW3762_DATA   sendFarme; //����֡����
    GDW3762_DATA   recvFarme; //��֡�����õĽ���֡����
    static unsigned char sendSeq;
	bool SendFrameIsAfn03F1;
};
unsigned char CalcFcs(unsigned char *ptr,unsigned short len);
int GDW3762_EncodeFn(unsigned char fn, unsigned char &dt1, unsigned char &dt2);
int GDW3762_DecodeFn(unsigned char dt1, unsigned char dt2, unsigned char &fn);
int GDW3762_CheckRecvFrame(unsigned char *recvbuf, int recvbuflen, int &offset, int &frameLen);

#endif

