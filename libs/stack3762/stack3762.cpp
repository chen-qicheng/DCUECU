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
        offset += 3; //偏移掉帧头和2字节长度字段
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
        offset += 3; //偏移掉帧头和2字节长度字段
        printf("FRAME TAIL flag not exist at right position maybe length bytes is error.\n");
        return GDW3762_FRAME_CHECK_ERROR;
    }
    //帧校验
    if (recvbuf[offset+length-2] != CalcFcs(recvbuf+offset+3, length - 5))
    {
        offset += length; //偏移掉整个帧
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
    sendFarme.dataLen += 2; // 这两位是长度值，暂时空置
    pTmp[sendFarme.dataLen++] = gdwSendFarmeInfo.Control.Val;
	SendFrameIsAfn03F1 = false;
    /* 信息域 */
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
    /* 地址域 */
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
    //pTmp[sendFarme.dataLen++] = (unsigned char)gdwSendFarmeInfo.dataUnit.dataLen; /* 没有长度字段 */
    if (0 != gdwSendFarmeInfo.dataUnit.dataLen)
    {
        memcpy(pTmp+sendFarme.dataLen, gdwSendFarmeInfo.dataUnit.data, gdwSendFarmeInfo.dataUnit.dataLen);
        sendFarme.dataLen += gdwSendFarmeInfo.dataUnit.dataLen; //现在只差fcs和tail 2个字节了
    }
    sendFarme.dataLen += 2;

    /* 回头填长度字段 */
    pTmp[1] = sendFarme.dataLen & 0xff;
    pTmp[2] = (sendFarme.dataLen >> 8) & 0xff;
    
    pTmp[sendFarme.dataLen - 2] = CalcFcs(pTmp+3, sendFarme.dataLen - 5);
    pTmp[sendFarme.dataLen - 1] = GDW3762_FRAME_TAIL_FLAG;

    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.AdParseRecvFrame
 Description  : 取缓存中尝试找到合法帧并解析
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
    int res=-1;     //默认失败，查找成功帧

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
			RelayCnt = 0;//上行
        }
    }
	

    /* 地址域 */
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

    gdwRecvFarmeInfo.dataUnit.dataLen = usDataLen - usOffset - 2; /*usOffset是数据单元前的长度，2是后面的fcs和tail flag长度 */
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
    /* down info 默认全0 */    
    gdwSendFarmeInfo.DownInfo.ReplyByteNum= 0xff;

    //gdwSendFarmeInfo.DownInfo.ReplyByteNum= 0x5f;
    //gdwSendFarmeInfo.DownInfo.seqNum = 0x22;
    return;
}

/*****************************************************************************
 函 数 名  : cGDW3762_FARME.FormAfn00F1
 功能描述  : 组帧AFN00F1
 输入参数  : GDW3762_PRM_E prm      取值参考 GDW3762_PRM_E
             unsigned char *data    可为NULL，代表使用默认值
             unsigned char dataLen  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月19日
    作    者   : fangxing
    修改内容   : 新生成函数

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
    /* 调用时填NULL则使用默认值 */
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
 函 数 名  : cGDW3762_FARME.FormAfn00F2
 功能描述  : 组帧AFN00F2
 输入参数  : GDW3762_PRM_E prm     
             GDW3762_NAK_ERROR_CODE_E errorCode
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月19日
    作    者   : fangxing
    修改内容   : 新生成函数

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
 函 数 名  : cGDW3762_FARME.FormAfn01
 功能描述  : 组帧AFN01
 输入参数  : unsigned char fn  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月19日
    作    者   : fangxing
    修改内容   : 新生成函数

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
 函 数 名  : cGDW3762_FARME.FormAfn02F1
 功能描述  : 组帧AFN02F1
 输入参数  : GDW3762_METER_PROTOCOL_E proTye   协议类型 请自行扩展
             unsigned char dataLen  报文长度
             unsigned char *data    报文内容
 
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年2月26日
    作    者   : fangxing
    修改内容   : 新生成函数

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
 函 数 名  : cGDW3762_FARME.FormAfn02F2
 功能描述  : 在AFN2 F1的基础上，扩展F2以适应长报文传输，内容长度字段改为2字
             节
 输入参数  : GDW3762_METER_PROTOCOL_E proTye  
             unsigned char dataLen            
             unsigned char *data              
             GDW3762_ADDR_S destAddr          
             GDW3762_ADDR_S srcAddr           
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年8月27日
    作    者   : fangxing
    修改内容   : 新生成函数

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
 函 数 名  : cGDW3762_FARME.FormAfn03
 功能描述  : 组帧AFN03
 输入参数  : unsigned char fn  目前不支持F3/F6，后续需要再扩展
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月19日
    作    者   : fangxing
    修改内容   : 新生成函数

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
 Description  : 扩展接口，读取表模块（从模块）版本号等信息
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
 Description  : 扩展功能码，用于实现G3模块私有参数的读取
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
 Description  : 载波从节点点名
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
 函 数 名  : cGDW3762_FARME.FormAfn05F3
 功能描述  : 组帧AFN05F3
 输入参数  : GDW3762_METER_PROTOCOL_E proTye   协议类型 请自行扩展
             unsigned char dataLen  报文长度
             unsigned char *data    报文内容
 
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018.11.7
    作    者   : zhangling
    修改内容   : 启动广播
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
	//广播升级，通信模块标识为0，见5.4.2 应用功能码 AFN
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
 Description  : 设置无线通讯参数
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
 Description  : 扩展功能码，用于配置G3的私有参数
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
 Description  : 新的标准中针对载波的调整指令，用于批量?-
                ?取节点拓扑信息
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
 函 数 名  : cGDW3762_FARME.FormAfn11F1
 功能描述  : 添加载波 从节点，完全由集中器控制，可以设定一次只增加一个节点，接口留好
 输入参数  : vector<GDW3762_AFN11F1_SLAVE_NODE_INFO_S> slaveNode
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月20日
    作    者   : fangxing
    修改内容   : 新生成函数

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
 函 数 名  : cGDW3762_FARME.FormAfn11F2
 功能描述  : 删除载波 从节点，完全由集中器控制，可以设定一次只删除一个节点，接口留好
 输入参数  : vector<GDW3762_ADDR_S> slaveNode
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月20日
    作    者   : fangxing
    修改内容   : 新生成函数

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
 Description  : 激活节点主动注册
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
 Description  : 关闭节点主动注册功能
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
 函 数 名  : cGDW3762_FARME.FormAfn12
 功能描述  : 组帧AFN03 用于暂停重启恢复路由
 输入参数  : unsigned char fn 
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月20日
    作    者   : fangxing
    修改内容   : 新生成函数

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
 Description  : 组帧：传输文件
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

    unsigned short dataUnitLen=0;       //临时存储长度变量
    
    //--将非文件数据内容先存入。因为文件数据内容实体不在该结构体中
    dataUnitLen=sizeof(GDW3762_AFN15F1_TRANS_FILE_S)-sizeof(unsigned char*);
    memcpy(gdwSendFarmeInfo.dataUnit.data, transFileSt, dataUnitLen);
    //--将文件数据实体存入
    if(0 < transFileSt->filePackLen)
    {
        memcpy(gdwSendFarmeInfo.dataUnit.data+dataUnitLen,transFileSt->filePackData,transFileSt->filePackLen);
        dataUnitLen += transFileSt->filePackLen;
    }

    //--修改协议栈对应数据区长度值
    gdwSendFarmeInfo.dataUnit.dataLen=dataUnitLen;

    memcpy(gdwSendFarmeInfo.DestAddr, destAddr, sizeof(GDW3762_ADDR_S));
    memcpy(gdwSendFarmeInfo.SrcAddr,  srcAddr,  sizeof(GDW3762_ADDR_S));
    
    FormSendFrame();
    return 0;
}

/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfnF0F21
 Description  : 扩展接口，组帧：查询文件处理进度
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
 Description  : 扩展接口，组帧：查询文件传输失败节点
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

    //--修改协议栈对应数据区长度值
    memcpy(gdwSendFarmeInfo.dataUnit.data,&startNodeNo,sizeof(unsigned short));
    gdwSendFarmeInfo.dataUnit.data[2]=nodeNum;
    
    //--修改协议栈对应数据区长度值
    gdwSendFarmeInfo.dataUnit.dataLen=3;
    
    FormSendFrame();
    return 0;
}


/*****************************************************************************
 Prototype    : cGDW3762_FARME.FormAfnF0F20
 Description  : 扩展接口，组帧：启动传输文件（支持广播及地址列表）
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







