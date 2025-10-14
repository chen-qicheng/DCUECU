#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "stack3762.h"

using namespace std;

unsigned char cGDW3762_FARME::sendSeq = 0;

int SLogbuf(unsigned char *pinbuff,const int buflen)
{
    if (buflen > 300) return 0;// big package can make this function wait and sagment fault
    char logbuf[1024];
    memset(logbuf,0,sizeof(logbuf));
    sprintf(logbuf, "Logbuf: ");
    int loglen  =   strlen(logbuf);
    for (int i = 0; i < buflen; ++i)
    {
        sprintf(logbuf + loglen + 3 * i, "%02x ", pinbuff[i]);
    }
    sprintf(logbuf + loglen + 3 * buflen, "\b\n");
    printf("%s,%s\n",__FILE__,logbuf);
    return 0;
}

int SLOGchar(unsigned char *outFrame,unsigned int outFrameLen)
{
    unsigned char FrameBuf[512] = {0};
    if (NULL==outFrame)
	return -1;
    int BufUsed = 0;

    for(unsigned int i = 0; i < outFrameLen; i++)
        BufUsed +=  sprintf((char *)(FrameBuf + BufUsed), " %c", outFrame[i]);

    sprintf((char *)(FrameBuf + BufUsed),"\n");

    printf("logbuf=%s",FrameBuf);
	return 0;
}

int GDW3762_CheckRecvFrame(unsigned char *recvbuf, int recvbuflen, int &offset, int &frameLen)
{
    offset = 0;
    int length;
    while (offset < recvbuflen)
    {
        if (GDW3762_FRAME_START_FLAG == recvbuf[offset])
        {
            break;
        }
        offset++;
    }
    if (recvbuflen-offset < 7)
    {
        return GDW3762_FRAME_CHECK_NOT_COMPLETE;
    }
    length = recvbuf[offset+1] + recvbuf[offset+2] * 256;
    if (length > GDW3762_MAX_FRAME_LEN)
    {
        offset += 3; //ƫ�Ƶ�֡ͷ��2�ֽڳ����ֶ�
        printf("length is more than %d, error frame.\n",GDW3762_MAX_FRAME_LEN);
        return GDW3762_FRAME_CHECK_ERROR;
    }
    if (offset+length > recvbuflen)
    {
        printf("length is more than recv buff length,need to continue receive.\n");
        return GDW3762_FRAME_CHECK_NOT_COMPLETE;
    }
    if (GDW3762_FRAME_TAIL_FLAG != recvbuf[offset+length-1])
    {
        offset += 3; //ƫ�Ƶ�֡ͷ��2�ֽڳ����ֶ�
        printf("FRAME TAIL flag not exist at right position maybe length bytes is error.\n");
        return GDW3762_FRAME_CHECK_ERROR;
    }
    //֡У��
    if (recvbuf[offset+length-2] != CalcFcs(recvbuf+offset+3, length - 5))
    {
        offset += length; //ƫ�Ƶ�����֡
        printf("FRAME CRC error.\n");
        return GDW3762_FRAME_CHECK_CRC_ERROR;
    }
    frameLen = length;
    return GDW3762_FRMAE_CHECK_SUCCESS;
    
}


unsigned char CalcFcs(unsigned char *ptr,unsigned short len)
{
	unsigned int iLoop;
    unsigned char cs=0;
	
    for(iLoop=0; iLoop<len; iLoop++)
	{
		cs += ptr[iLoop];
	}
	return cs;
}

int GDW3762_EncodeFn(unsigned char fn, unsigned char &dt1, unsigned char &dt2)
{
    if (fn > 248)
    {
        return -1;
    }
    unsigned char tmp = fn - 1;
    dt2 = (tmp >> 3) & 0x1f;
    dt1 = 1 << (tmp & 0x7);
    return 0;
}


int GDW3762_DecodeFn(unsigned char dt1, unsigned char dt2, unsigned char &fn)
{
    if ((dt2 > 30) || (0 == dt1))
    {
        return -1;
    }
    fn = (dt2 << 3) & 0xf8;
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        if (1 == ((dt1 >> i) & 1))
        {
            fn += i+1;
            break;
        }
    }
    return 0;
}

cGDW3762_FARME::cGDW3762_FARME(void)
{
    
}

cGDW3762_FARME::~cGDW3762_FARME(void)
{
    
}


int cGDW3762_FARME::FormSendFrame()
{
    unsigned char AddrExistFlag = 0;
    unsigned char RelayCnt = 0;
    memset(&sendFarme, 0, sizeof(sendFarme));
    unsigned char *pTmp = sendFarme.data;
    pTmp[sendFarme.dataLen++] = GDW3762_FRAME_START_FLAG;
    sendFarme.dataLen += 2; // ����λ�ǳ���ֵ����ʱ����
    pTmp[sendFarme.dataLen++] = gdwSendFarmeInfo.Control.Val;
	SendFrameIsAfn03F1 = false;
    /* ��Ϣ�� */
    if (GDW3762_DIR_DOWN == gdwSendFarmeInfo.Control.bits.Dir)
    {
        //printf("sendFarme.dataLen=%d\n", sendFarme.dataLen);
        memcpy(pTmp+sendFarme.dataLen, &gdwSendFarmeInfo.DownInfo, sizeof(GDW3762_DOWN_INFOR_S));
        sendFarme.dataLen += sizeof(GDW3762_DOWN_INFOR_S);
        //printf("R zone size if %d, sendFarme.dataLen=%d, %d, %d\n", sizeof(GDW3762_DOWN_INFOR_S), sendFarme.dataLen,
                                        //sizeof(GDW3762_DOWNLINK_COMM_PARAM_U), sizeof(unsigned short));
        //for (int i = 0; i < sendFarme.dataLen; i++)
        //{
        //    printf("%02x ", pTmp[i]);
        //}
        //printf("\n");
        
        if (0 != gdwSendFarmeInfo.DownInfo.LinkParam.bits.CommModuleFlag)
        {
            AddrExistFlag = 1;
            RelayCnt = gdwSendFarmeInfo.DownInfo.LinkParam.bits.RelayLevel;
        }
    }
    else if (GDW3762_DIR_UP == gdwSendFarmeInfo.Control.bits.Dir)
    {
        memcpy(pTmp+sendFarme.dataLen, &gdwSendFarmeInfo.UpInfo, sizeof(GDW3762_UP_INFOR_S));
        sendFarme.dataLen += sizeof(GDW3762_UP_INFOR_S);
        if (0 != gdwSendFarmeInfo.UpInfo.LinkParam.bits.CommModuleFlag)
        {
            AddrExistFlag = 1;
            RelayCnt = gdwSendFarmeInfo.UpInfo.LinkParam.bits.RelayLevel;
        }
    }
    /* ��ַ�� */
    if (AddrExistFlag)
    {
        memcpy(pTmp+sendFarme.dataLen, gdwSendFarmeInfo.SrcAddr, GDW3762_MAC_ADDR_LEN);
        sendFarme.dataLen += GDW3762_MAC_ADDR_LEN;
        if (RelayCnt)
        {
            for (int i = 0; i < RelayCnt; i++)
            {
                memcpy(pTmp+sendFarme.dataLen, &gdwSendFarmeInfo.relayAddr[i], GDW3762_MAC_ADDR_LEN);
                sendFarme.dataLen += GDW3762_MAC_ADDR_LEN;
            }
        }
        memcpy(pTmp+sendFarme.dataLen, gdwSendFarmeInfo.DestAddr, GDW3762_MAC_ADDR_LEN);
        sendFarme.dataLen += GDW3762_MAC_ADDR_LEN;
    }

    /* afn fn */
    pTmp[sendFarme.dataLen++] = gdwSendFarmeInfo.afn;
    GDW3762_EncodeFn(gdwSendFarmeInfo.fn, gdwSendFarmeInfo.dt.dt1, gdwSendFarmeInfo.dt.dt2);
    pTmp[sendFarme.dataLen++] = gdwSendFarmeInfo.dt.dt1;
    pTmp[sendFarme.dataLen++] = gdwSendFarmeInfo.dt.dt2;
    //pTmp[sendFarme.dataLen++] = (unsigned char)gdwSendFarmeInfo.dataUnit.dataLen; /* û�г����ֶ� */
    if (0 != gdwSendFarmeInfo.dataUnit.dataLen)
    {
        memcpy(pTmp+sendFarme.dataLen, gdwSendFarmeInfo.dataUnit.data, gdwSendFarmeInfo.dataUnit.dataLen);
        sendFarme.dataLen += gdwSendFarmeInfo.dataUnit.dataLen; //����ֻ��fcs��tail 2���ֽ���
    }
    sendFarme.dataLen += 2;

    /* ��ͷ����ֶ� */
    pTmp[1] = sendFarme.dataLen & 0xff;
    pTmp[2] = (sendFarme.dataLen >> 8) & 0xff;
    
    pTmp[sendFarme.dataLen - 2] = CalcFcs(pTmp+3, sendFarme.dataLen - 5);
    pTmp[sendFarme.dataLen - 1] = GDW3762_FRAME_TAIL_FLAG;

    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.AdParseRecvFrame
 Description  : ȡ�����г����ҵ��Ϸ�֡������
 Input        : uint8_t *pInBuf  
                int bufLen       
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2019/4/8
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::AdParseRecvFrame(uint8_t *pInBuf,int bufLen)
{
    int res=-1;     //Ĭ��ʧ�ܣ����ҳɹ�֡

    for (int i = 0; i < bufLen; ++i)
    {
        if(pInBuf[i]!=0x68)continue;
        
        for(int j=bufLen-1; j>i;--j)
        {
            if(pInBuf[j]!=0x16)continue;
            
            recvFarme.dataLen=j-i+1;
            memcpy(recvFarme.data,pInBuf+i,recvFarme.dataLen);
            if(ParseRecvFrame()==0)
            {
                res = 0;
            }
            
        }
    }


    return res;
}



int cGDW3762_FARME::ParseRecvFrame()
{
    //SLogbuf(recvFarme.data, recvFarme.dataLen);
    memset(&gdwRecvFarmeInfo, 0, sizeof(gdwRecvFarmeInfo));
    
    unsigned char *pTmp = recvFarme.data;
    if (recvFarme.dataLen > GDW3762_MAX_FRAME_LEN)
    {
        return -1;
    }
    if ( (GDW3762_FRAME_START_FLAG != pTmp[0])
      || (GDW3762_FRAME_TAIL_FLAG != pTmp[recvFarme.dataLen - 1]) )
    {
        return -2;
    }
    
    unsigned short usDataLen = (pTmp[2] << 8) | pTmp[1];
    if (recvFarme.dataLen != usDataLen)
    {
        return -3;
    }
    if (pTmp[recvFarme.dataLen - 2] != CalcFcs(pTmp + 3, usDataLen - 5))
    {
        return -4;
    }
    unsigned short usOffset = 3;
    gdwRecvFarmeInfo.Control.Val= pTmp[usOffset++];
    
    unsigned char AddrExistFlag = 0;
    unsigned char RelayCnt = 0;
    if (GDW3762_DIR_DOWN == gdwRecvFarmeInfo.Control.bits.Dir)
    {
        memcpy(&gdwRecvFarmeInfo.DownInfo, pTmp + usOffset, sizeof(GDW3762_DOWN_INFOR_S));
        usOffset += sizeof(GDW3762_DOWN_INFOR_S);
        if (0 != gdwRecvFarmeInfo.DownInfo.LinkParam.bits.CommModuleFlag)
        {
            AddrExistFlag = 1;
            RelayCnt = gdwRecvFarmeInfo.DownInfo.LinkParam.bits.RelayLevel;
			printf("########Down############RelayCnt=%d\n",RelayCnt);
        }
    }
    else if (GDW3762_DIR_UP == gdwRecvFarmeInfo.Control.bits.Dir)
    {
        memcpy(&gdwRecvFarmeInfo.UpInfo, pTmp+usOffset, sizeof(GDW3762_UP_INFOR_S));
        usOffset += sizeof(GDW3762_UP_INFOR_S);
        if (0 != gdwRecvFarmeInfo.UpInfo.LinkParam.bits.CommModuleFlag)
        {
            AddrExistFlag = 1;
            RelayCnt = gdwRecvFarmeInfo.UpInfo.LinkParam.bits.RelayLevel;
			printf("########Up############RelayCnt=%d\n",RelayCnt);
			RelayCnt = 0;//����
        }
    }
	

    /* ��ַ�� */
    if (AddrExistFlag)
    {
        memcpy(gdwRecvFarmeInfo.SrcAddr, pTmp+usOffset, GDW3762_MAC_ADDR_LEN);
        usOffset += GDW3762_MAC_ADDR_LEN;
        if (RelayCnt)
        {
            for (int i = 0; i < RelayCnt; i++)
            {
                memcpy(&gdwRecvFarmeInfo.relayAddr[i], pTmp+usOffset, GDW3762_MAC_ADDR_LEN);
                usOffset += GDW3762_MAC_ADDR_LEN;
            }			
        }
        memcpy(gdwRecvFarmeInfo.DestAddr, pTmp+usOffset, GDW3762_MAC_ADDR_LEN);
        usOffset += GDW3762_MAC_ADDR_LEN;
    }

    /* afn fn */
    gdwRecvFarmeInfo.afn = pTmp[usOffset++];
    gdwRecvFarmeInfo.dt.dt1 = pTmp[usOffset++];
    gdwRecvFarmeInfo.dt.dt2 = pTmp[usOffset++];
    if (0 != GDW3762_DecodeFn(gdwRecvFarmeInfo.dt.dt1,gdwRecvFarmeInfo.dt.dt2, gdwRecvFarmeInfo.fn))
    {
        return -5;
    }
    
    if (usDataLen < usOffset + 2)
    {
        return -6;
    }
    //printf("afn=%d, fn=%d\n",gdwRecvFarmeInfo.afn, gdwRecvFarmeInfo.fn);

    gdwRecvFarmeInfo.dataUnit.dataLen = usDataLen - usOffset - 2; /*usOffset�����ݵ�Ԫǰ�ĳ��ȣ�2�Ǻ����fcs��tail flag���� */
    //printf("usOffset=%d, dataLen=%d\n",usOffset, gdwRecvFarmeInfo.dataUnit.dataLen);
    memcpy(gdwRecvFarmeInfo.dataUnit.data, pTmp+usOffset, gdwRecvFarmeInfo.dataUnit.dataLen);
    //SLogbuf(gdwRecvFarmeInfo.dataUnit.data, gdwRecvFarmeInfo.dataUnit.dataLen);

    return 0;
}

void cGDW3762_FARME::SetSendFrameDefaultParam()
{
    memset(&gdwSendFarmeInfo, 0, sizeof(GDW3762_FARME_INFO_S));
    gdwSendFarmeInfo.Control.Val= 0x41;
    
    //gdwSendFarmeInfo.Control.Val= 0x4A;
    /* down info Ĭ��ȫ0 */    
    gdwSendFarmeInfo.DownInfo.ReplyByteNum= 0xff;

    //gdwSendFarmeInfo.DownInfo.ReplyByteNum= 0x5f;
    //gdwSendFarmeInfo.DownInfo.seqNum = 0x22;
    return;
}

/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn00F1
 ��������  : ��֡AFN00F1
 �������  : GDW3762_PRM_E prm      ȡֵ�ο� GDW3762_PRM_E
             unsigned char *data    ��ΪNULL������ʹ��Ĭ��ֵ
             unsigned char dataLen  
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��19��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn00F1(GDW3762_PRM_E prm, unsigned char *data, unsigned char dataLen,unsigned char seqnum)
{
    SetSendFrameDefaultParam();
	gdwSendFarmeInfo.DownInfo.seqNum = seqnum;
    gdwSendFarmeInfo.Control.bits.Prm = prm;
    gdwSendFarmeInfo.afn = 0;
    gdwSendFarmeInfo.fn = 1;
	
    if (NULL != data)
    {
        if (GDW3762_AFN00FN1_DATALEN != dataLen)
        {
            return -1;
        }
        memcpy(&gdwSendFarmeInfo.dataUnit.data, data, dataLen);
    }
    /* ����ʱ��NULL��ʹ��Ĭ��ֵ */
    else
    {
        gdwSendFarmeInfo.dataUnit.data[0] = 0xff;
        gdwSendFarmeInfo.dataUnit.data[1] = 0xff;
        gdwSendFarmeInfo.dataUnit.data[2] = 0;
        gdwSendFarmeInfo.dataUnit.data[3] = 0;
    }
    gdwSendFarmeInfo.dataUnit.dataLen = GDW3762_AFN00FN1_DATALEN;
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn00F2
 ��������  : ��֡AFN00F2
 �������  : GDW3762_PRM_E prm     
             GDW3762_NAK_ERROR_CODE_E errorCode
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��19��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn00F2(GDW3762_PRM_E prm, GDW3762_NAK_ERROR_CODE_E errorCode)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.Control.bits.Prm = prm;
    gdwSendFarmeInfo.afn = 0;
    gdwSendFarmeInfo.fn = 2;

    gdwSendFarmeInfo.dataUnit.data[0] = errorCode;
    gdwSendFarmeInfo.dataUnit.dataLen = GDW3762_AFN00FN2_DATALEN;
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn01
 ��������  : ��֡AFN01
 �������  : unsigned char fn  
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��19��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn01(unsigned char fn)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 1;
    gdwSendFarmeInfo.fn = fn;

    FormSendFrame();
    return 0;
}

/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn02F1
 ��������  : ��֡AFN02F1
 �������  : GDW3762_METER_PROTOCOL_E proTye   Э������ ��������չ
             unsigned char dataLen  ���ĳ���
             unsigned char *data    ��������
 
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��2��26��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn02F1(GDW3762_METER_PROTOCOL_E proTye, unsigned short dataLen, unsigned char *data,
                                GDW3762_ADDR_S destAddr, GDW3762_ADDR_S srcAddr)
{
    if ( (0 == dataLen) || (dataLen > GDW3762_MAX_FRAME_LEN-2 ) )
    {
        return -1;
    }
    
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 2;
    gdwSendFarmeInfo.fn = 1;
    gdwSendFarmeInfo.DownInfo.LinkParam.bits.CommModuleFlag = 1;
    memcpy(gdwSendFarmeInfo.DestAddr, destAddr, sizeof(GDW3762_ADDR_S));
    memcpy(gdwSendFarmeInfo.SrcAddr,  srcAddr,  sizeof(GDW3762_ADDR_S));
    
    gdwSendFarmeInfo.dataUnit.dataLen = dataLen + 2;
    gdwSendFarmeInfo.dataUnit.data[0] = proTye;
    gdwSendFarmeInfo.dataUnit.data[1] = dataLen;
    memcpy(gdwSendFarmeInfo.dataUnit.data + 2, data, dataLen);
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn02F2
 ��������  : ��AFN2 F1�Ļ����ϣ���չF2����Ӧ�����Ĵ��䣬���ݳ����ֶθ�Ϊ2��
             ��
 �������  : GDW3762_METER_PROTOCOL_E proTye  
             unsigned char dataLen            
             unsigned char *data              
             GDW3762_ADDR_S destAddr          
             GDW3762_ADDR_S srcAddr           
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��8��27��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn02F2(GDW3762_METER_PROTOCOL_E proTye, unsigned short dataLen, unsigned char *data,
                                GDW3762_ADDR_S destAddr, GDW3762_ADDR_S srcAddr)
{
    if ( (0 == dataLen) || (dataLen > GDW3762_MAX_FRAME_LEN-2 ) )
    {
        return -1;
    }
    
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 2;
    gdwSendFarmeInfo.fn = 2;
    gdwSendFarmeInfo.DownInfo.LinkParam.bits.CommModuleFlag = 1;
    memcpy(gdwSendFarmeInfo.DestAddr, destAddr, sizeof(GDW3762_ADDR_S));
    memcpy(gdwSendFarmeInfo.SrcAddr,  srcAddr,  sizeof(GDW3762_ADDR_S));
    
    gdwSendFarmeInfo.dataUnit.dataLen = dataLen + 3;
    gdwSendFarmeInfo.dataUnit.data[0] = proTye;
    gdwSendFarmeInfo.dataUnit.data[1] = dataLen & 0xff;
    gdwSendFarmeInfo.dataUnit.data[2] = (dataLen >> 8) & 0xff;
    
    memcpy(gdwSendFarmeInfo.dataUnit.data + 3, data, dataLen);
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn03
 ��������  : ��֡AFN03
 �������  : unsigned char fn  Ŀǰ��֧��F3/F6��������Ҫ����չ
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��19��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn03(unsigned char fn)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 3;
    gdwSendFarmeInfo.fn = fn;

    FormSendFrame();
	if(fn == 1)
	{
		SendFrameIsAfn03F1= true;
	}
    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn03F21
 Description  : ��չ�ӿڣ���ȡ��ģ�飨��ģ�飩�汾�ŵ���Ϣ
 Input        : GDW3762_ADDR_S addr  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/10/19
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn03F21(GDW3762_ADDR_S addr)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0X03;
    gdwSendFarmeInfo.fn = 21;

    memcpy(gdwSendFarmeInfo.dataUnit.data, addr, sizeof(GDW3762_ADDR_S));
    gdwSendFarmeInfo.dataUnit.dataLen = sizeof(GDW3762_ADDR_S);

    FormSendFrame();
    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn03F100
 Description  : ��չ�����룬����ʵ��G3ģ��˽�в����Ķ�ȡ
 Input        : unsigned char * mBuf   
                unsigned short bufLen  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2019/3/22
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn03F100(unsigned char * mBuf,unsigned short bufLen)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x03;
    gdwSendFarmeInfo.fn = 100;

    memcpy(gdwSendFarmeInfo.dataUnit.data, mBuf, bufLen);
    gdwSendFarmeInfo.dataUnit.dataLen = bufLen;
    
    FormSendFrame();
    return 0;
}

int cGDW3762_FARME::FormAfn03F102()
{
	SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x03;
    gdwSendFarmeInfo.fn = 102;

    FormSendFrame();
    return 0;
	
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn04F2
 Description  : �ز��ӽڵ����
 Input        : GDW3762_ADDR_S addr  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/11/9
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn04F2(GDW3762_ADDR_S srcAddr,GDW3762_ADDR_S addr)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0X04;
    gdwSendFarmeInfo.fn = 2;

    gdwSendFarmeInfo.Control.Val=0x43;
    gdwSendFarmeInfo.DownInfo.LinkParam.Val=0x04 + 0x00*0xff;
    gdwSendFarmeInfo.DownInfo.ReplyByteNum=0xff;
    gdwSendFarmeInfo.DownInfo.CommSpeedUnit=0;
    gdwSendFarmeInfo.DownInfo.CommSpeedValue=0;
    gdwSendFarmeInfo.DownInfo.seqNum=0;

    memcpy(gdwSendFarmeInfo.SrcAddr,srcAddr,sizeof(GDW3762_ADDR_S));
    memcpy(gdwSendFarmeInfo.DestAddr, addr, sizeof(GDW3762_ADDR_S));

    FormSendFrame();
    return 0;
}

int cGDW3762_FARME::FormAfn04F3(GDW3762_ADDR_S srcAddr,GDW3762_ADDR_S addr,unsigned char cnt)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0X04;
    gdwSendFarmeInfo.fn = 3;

    gdwSendFarmeInfo.Control.Val=0x4A;
    //gdwSendFarmeInfo.DownInfo.LinkParam.Val=0x04 + 0x00*0xff;
    gdwSendFarmeInfo.DownInfo.ReplyByteNum=0x5f;
    gdwSendFarmeInfo.DownInfo.CommSpeedUnit=0;
    gdwSendFarmeInfo.DownInfo.CommSpeedValue=0;
    gdwSendFarmeInfo.DownInfo.seqNum=0;
	
	gdwSendFarmeInfo.dataUnit.data[0]=cnt;
	memcpy(gdwSendFarmeInfo.dataUnit.data + 1, addr, sizeof(GDW3762_ADDR_S));
	gdwSendFarmeInfo.dataUnit.data[sizeof(GDW3762_ADDR_S) + 1]=0;
	gdwSendFarmeInfo.dataUnit.data[sizeof(GDW3762_ADDR_S) + 2]=1;
	gdwSendFarmeInfo.dataUnit.data[sizeof(GDW3762_ADDR_S) + 3]=0;
    gdwSendFarmeInfo.dataUnit.dataLen = sizeof(GDW3762_ADDR_S) + 4;
    //memcpy(gdwSendFarmeInfo.SrcAddr,srcAddr,sizeof(GDW3762_ADDR_S));
    //memcpy(gdwSendFarmeInfo.DestAddr, addr, sizeof(GDW3762_ADDR_S));

    FormSendFrame();
    return 0;
}

int cGDW3762_FARME::FormAfn05F1(GDW3762_ADDR_S addr)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x05;
    gdwSendFarmeInfo.fn = 1;

    memcpy(gdwSendFarmeInfo.dataUnit.data, addr, sizeof(GDW3762_ADDR_S));
    gdwSendFarmeInfo.dataUnit.dataLen = sizeof(GDW3762_ADDR_S);
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn05F3
 ��������  : ��֡AFN05F3
 �������  : GDW3762_METER_PROTOCOL_E proTye   Э������ ��������չ
             unsigned char dataLen  ���ĳ���
             unsigned char *data    ��������
 
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018.11.7
    ��    ��   : zhangling
    �޸�����   : �����㲥
*****************************************************************************/
int cGDW3762_FARME::FormAfn05F3(GDW3762_METER_PROTOCOL_E proTye, 
unsigned short dataLen, unsigned char *data)
{
    if ( (0 == dataLen) || (dataLen > GDW3762_MAX_FRAME_LEN-2 ) )
    {
        return -1;
    }
    
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 5;
    gdwSendFarmeInfo.fn = 3;
	//�㲥������ͨ��ģ���ʶΪ0����5.4.2 Ӧ�ù����� AFN
    gdwSendFarmeInfo.DownInfo.LinkParam.bits.CommModuleFlag = 0;
    
	gdwSendFarmeInfo.dataUnit.dataLen = dataLen + 3;
    gdwSendFarmeInfo.dataUnit.data[0] = proTye;
    gdwSendFarmeInfo.dataUnit.data[1] = dataLen & 0xff;
    gdwSendFarmeInfo.dataUnit.data[2] = (dataLen >> 8) & 0xff;
    memcpy(gdwSendFarmeInfo.dataUnit.data + 3, data, dataLen);
    
    FormSendFrame();
    return 0;

    
}

int cGDW3762_FARME::FormAfn05F4(int mValue)
{
	SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 5;
    gdwSendFarmeInfo.fn = 4;

	if(mValue == 1)
	{
		gdwSendFarmeInfo.dataUnit.data[0] = 0x5F;
		gdwSendFarmeInfo.dataUnit.dataLen = 1;
	}
	else if (mValue == 0)
	{
		gdwSendFarmeInfo.dataUnit.data[0] = 0x5A;
		gdwSendFarmeInfo.dataUnit.dataLen = 1;

	}
	FormSendFrame();
    return 0;
}



/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn05F5
 Description  : ��������ͨѶ����
 Input        : GDW3762_ADDR_S addr  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/11/9
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn05F5(GDW3762_AFN05F5_PARAM_S mParam)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x05;
    gdwSendFarmeInfo.fn = 5;

    memcpy(gdwSendFarmeInfo.dataUnit.data, &mParam, sizeof(GDW3762_AFN05F5_PARAM_S));
    gdwSendFarmeInfo.dataUnit.dataLen = sizeof(GDW3762_AFN05F5_PARAM_S);
    
    FormSendFrame();
    return 0;
}



/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn05F100
 Description  : ��չ�����룬��������G3��˽�в���
 Input        : unsigned char * mBuf   
                unsigned short bufLen  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2019/3/22
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn05F100(unsigned char * mBuf,unsigned short bufLen)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x05;
    gdwSendFarmeInfo.fn = 100;

    memcpy(gdwSendFarmeInfo.dataUnit.data, mBuf, bufLen);
    gdwSendFarmeInfo.dataUnit.dataLen = bufLen;
    
    FormSendFrame();
    return 0;
}

int cGDW3762_FARME::FormAfn10F1()
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x10;
    gdwSendFarmeInfo.fn = 1;
    
    FormSendFrame();
    return 0;
}


int cGDW3762_FARME::FormAfn10F2(GDW3762_AFN10F2_DOWN_DATA_S *data)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x10;
    gdwSendFarmeInfo.fn = 2;

    memcpy(gdwSendFarmeInfo.dataUnit.data, data, sizeof(GDW3762_AFN10F2_DOWN_DATA_S));
    gdwSendFarmeInfo.dataUnit.dataLen = sizeof(GDW3762_AFN10F2_DOWN_DATA_S);
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn10F21
 Description  : �µı�׼������ز��ĵ���ָ���������?-
                ?ȡ�ڵ�������Ϣ
 Input        : GDW3762_AFN10F2_DOWN_DATA_S *data  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/11/14
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn10F21(short nodeStartIdx,unsigned char nodeNum)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x10;
    gdwSendFarmeInfo.fn = 21;

    gdwSendFarmeInfo.dataUnit.data[0]=(unsigned char)(nodeStartIdx&0xFF);
    gdwSendFarmeInfo.dataUnit.data[1]=(unsigned char)((nodeStartIdx>>8)&0xFF);
    gdwSendFarmeInfo.dataUnit.data[2]=nodeNum;
    gdwSendFarmeInfo.dataUnit.dataLen = 3;
    
    FormSendFrame();
    return 0;
}


/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn11F1
 ��������  : ����ز� �ӽڵ㣬��ȫ�ɼ��������ƣ������趨һ��ֻ����һ���ڵ㣬�ӿ�����
 �������  : vector<GDW3762_AFN11F1_SLAVE_NODE_INFO_S> slaveNode
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��20��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn11F1(vector <GDW3762_AFN11F1_SLAVE_NODE_INFO_S> slaveNode)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x11;
    gdwSendFarmeInfo.fn = 1;

    gdwSendFarmeInfo.dataUnit.data[0] = 1;
    memcpy(gdwSendFarmeInfo.dataUnit.data+1, &slaveNode[0], sizeof(GDW3762_AFN11F1_SLAVE_NODE_INFO_S));
    gdwSendFarmeInfo.dataUnit.dataLen = 1 + sizeof(GDW3762_AFN11F1_SLAVE_NODE_INFO_S);
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn11F2
 ��������  : ɾ���ز� �ӽڵ㣬��ȫ�ɼ��������ƣ������趨һ��ֻɾ��һ���ڵ㣬�ӿ�����
 �������  : vector<GDW3762_ADDR_S> slaveNode
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��20��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn11F2(vector <GDW3762_ADDR_S> slaveNode)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x11;
    gdwSendFarmeInfo.fn = 2;

    gdwSendFarmeInfo.dataUnit.data[0] = 1;
    memcpy(gdwSendFarmeInfo.dataUnit.data+1, &slaveNode[0], sizeof(GDW3762_ADDR_S));
    gdwSendFarmeInfo.dataUnit.dataLen = 1 + sizeof(GDW3762_ADDR_S);
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn11F5
 Description  : ����ڵ�����ע��
 Input        : GDW3762_AFN11F5_SLAVE_AUTO_REG_S *regInfo  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2019/3/25
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn11F5(GDW3762_AFN11F5_SLAVE_AUTO_REG_S *regInfo)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x11;
    gdwSendFarmeInfo.fn = 5;

    memcpy(gdwSendFarmeInfo.dataUnit.data, regInfo, sizeof(GDW3762_AFN11F5_SLAVE_AUTO_REG_S));
    gdwSendFarmeInfo.dataUnit.dataLen = sizeof(GDW3762_AFN11F5_SLAVE_AUTO_REG_S);
    
    FormSendFrame();
    return 0;
}


/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn11F5
 Description  : �رսڵ�����ע�Ṧ��
 Input        : None
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2019/3/25
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn11F6(void)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x11;
    gdwSendFarmeInfo.fn = 6;

    FormSendFrame();
    return 0;

}
/*****************************************************************************
 �� �� ��  : cGDW3762_FARME.FormAfn12
 ��������  : ��֡AFN03 ������ͣ�����ָ�·��
 �������  : unsigned char fn 
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��20��
    ��    ��   : fangxing
    �޸�����   : �����ɺ���

*****************************************************************************/
int cGDW3762_FARME::FormAfn12(unsigned char fn)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x12;
    gdwSendFarmeInfo.fn = fn;

    FormSendFrame();
    return 0;
}

int cGDW3762_FARME::FormAfn13F1(GDW3762_METER_PROTOCOL_E meterPro, vector <GDW3762_ADDR_S> meterList,
    unsigned char *pData, unsigned short dataLen)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x13;
    gdwSendFarmeInfo.fn = 1;

    unsigned char *pTmp = gdwSendFarmeInfo.dataUnit.data;
    *pTmp++ = meterPro;
    *pTmp++ = meterList.size();
    for (unsigned int i = 0; i < meterList.size(); i++)
    {
        memcpy(pTmp, &meterList[i], sizeof(GDW3762_ADDR_S));
        pTmp+= sizeof(GDW3762_ADDR_S);
    }
    *pTmp++ = dataLen;
    memcpy(pTmp, pData, dataLen);
    pTmp += dataLen;
    gdwSendFarmeInfo.dataUnit.dataLen = pTmp - gdwSendFarmeInfo.dataUnit.data;
    
    FormSendFrame();
    return 0;
}

int cGDW3762_FARME::FormAfn13F2(GDW3762_METER_PROTOCOL_E meterPro, vector <GDW3762_ADDR_S> meterList,
    unsigned char *pData, unsigned short dataLen)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x13;
    gdwSendFarmeInfo.fn = 2;

    unsigned char *pTmp = gdwSendFarmeInfo.dataUnit.data;
    *pTmp++ = meterPro;
    *pTmp++ = meterList.size();
    for (unsigned int i = 0; i < meterList.size(); i++)
    {
        memcpy(pTmp, &meterList[i], sizeof(GDW3762_ADDR_S));
        pTmp+= sizeof(GDW3762_ADDR_S);
    }
    *pTmp++ = dataLen & 0xff;
    *pTmp++ = (dataLen >> 8) & 0xff;
    memcpy(pTmp, pData, dataLen);
    pTmp += dataLen;
    gdwSendFarmeInfo.dataUnit.dataLen = pTmp - gdwSendFarmeInfo.dataUnit.data;
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn15F1
 Description  : ��֡�������ļ�
 Input        : tagGDW3762_AFN15F1_TRANS_FILE *transFileSt  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/9/12
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfn15F1(GDW3762_AFN15F1_TRANS_FILE_S *transFileSt,GDW3762_ADDR_S destAddr, GDW3762_ADDR_S srcAddr)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0x15;
    gdwSendFarmeInfo.fn = 1;

    unsigned short dataUnitLen=0;       //��ʱ�洢���ȱ���
    
    //--�����ļ����������ȴ��롣��Ϊ�ļ���������ʵ�岻�ڸýṹ����
    dataUnitLen=sizeof(GDW3762_AFN15F1_TRANS_FILE_S)-sizeof(unsigned char*);
    memcpy(gdwSendFarmeInfo.dataUnit.data, transFileSt, dataUnitLen);
    //--���ļ�����ʵ�����
    if(0 < transFileSt->filePackLen)
    {
        memcpy(gdwSendFarmeInfo.dataUnit.data+dataUnitLen,transFileSt->filePackData,transFileSt->filePackLen);
        dataUnitLen += transFileSt->filePackLen;
    }

    //--�޸�Э��ջ��Ӧ����������ֵ
    gdwSendFarmeInfo.dataUnit.dataLen=dataUnitLen;

    memcpy(gdwSendFarmeInfo.DestAddr, destAddr, sizeof(GDW3762_ADDR_S));
    memcpy(gdwSendFarmeInfo.SrcAddr,  srcAddr,  sizeof(GDW3762_ADDR_S));
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfnF0F21
 Description  : ��չ�ӿڣ���֡����ѯ�ļ��������
 Input        : void  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/9/19
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfnF0F21(void)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0xF0;
    gdwSendFarmeInfo.fn = 0x21;
    
    FormSendFrame();
    return 0;
}


/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfn00F22
 Description  : ��չ�ӿڣ���֡����ѯ�ļ�����ʧ�ܽڵ�
 Input        : unsigned short startNodeNo  
                unsigned char nodeNum       
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/9/19
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfnF0F22(unsigned short startNodeNo, unsigned char nodeNum)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0xF0;
    gdwSendFarmeInfo.fn = 22;

    //--�޸�Э��ջ��Ӧ����������ֵ
    memcpy(gdwSendFarmeInfo.dataUnit.data,&startNodeNo,sizeof(unsigned short));
    gdwSendFarmeInfo.dataUnit.data[2]=nodeNum;
    
    //--�޸�Э��ջ��Ӧ����������ֵ
    gdwSendFarmeInfo.dataUnit.dataLen=3;
    
    FormSendFrame();
    return 0;
}


/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfnF0F20
 Description  : ��չ�ӿڣ���֡�����������ļ���֧�ֹ㲥����ַ�б�
 Input        : unsigned char *pInBuff      
                int inBuffLen  
 Output       : None
 Return Value : int
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/9/19
    Author       : WILLEN
    Modification : Created function

*****************************************************************************/
int cGDW3762_FARME::FormAfnF0F23(unsigned char *pInBuff,int inBuffLen)
{
    SetSendFrameDefaultParam();
    gdwSendFarmeInfo.afn = 0xF0;
    gdwSendFarmeInfo.fn = 23;

    memcpy(gdwSendFarmeInfo.dataUnit.data,pInBuff,inBuffLen);
    
    gdwSendFarmeInfo.dataUnit.dataLen=inBuffLen;
    
    FormSendFrame();
    return 0;
}







