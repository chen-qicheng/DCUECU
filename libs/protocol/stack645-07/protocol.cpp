
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h> 
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <ctype.h>
#include "Func6513.h"
#include "protocol.h"
//#include "Logger.h"


u8 MeterAddr[6];
CProtocol::CProtocol( )
{

}


CProtocol::~CProtocol( )
{
}


void CProtocol::Init(u8 serial_num , s32 buad )
{
    S_COMM_PARA com_para;
    com_para.flag           = 1;                    //有效性
    com_para.baudrate       = buad;                 //速率
    com_para.serial_num     = serial_num;           //端口号enum COM_NO { COM1=0, COM2=1, ... };
    com_para.parity         = PARITY_NONE;          //校验
    com_para.sync_mode      = COM_BLOCK;            //同步方式
    com_para.rtscts         = COM_RTSCTS_DISABLE;   //流控制
    com_para.bytesize       = COM_BYTESIZE8;        //数据位长度
    com_para.stopbits       = COM_STOPBITS1;        //停止位长度
    m_AsyncCom.Init( com_para );
}




s32 CProtocol::Get( u8 *buff, s32 Len, u8 cmd, s32 addr )
{
    s32 i;
    u8 queryFrm[ 300 ] = { 0 };
    u8 outFrm[ 300 ] = { 0 };
    s32 trys = 0;
    for( i = 0; i < Len; ) 
    {
        s32 outLen = sizeof( outFrm );
        s32 queryLen = FormQueryFrm( queryFrm, cmd, addr + i );
        if( queryLen <= 0 ) break;
        if( WriteCom( queryFrm, queryLen, outFrm, outLen, true) )
        {
            s32 valueLen = 0;
            if( ProcReturnedFrame( outFrm, outLen, addr + i, buff + i, valueLen) == 0  )
            {
                if( valueLen > 0 )
                {
                    trys = 0;
                    i += valueLen;
                    continue;
                }
            }
        }
        trys ++;
        if( trys > 2 )
        {
            break;
        } 
    }
    return i;
}


void CProtocol::SetClock( )
{
    u8 buff[ 64 ] = { 0 };
    s32 queryLen = FormCheckClock( buff );
    u8 outFrm[ 255] = { 0 };
    s32 outLen = 255;
    WriteCom( buff, queryLen, outFrm, outLen, true );
}


s32 CProtocol::Set( u8 *buff, s32 Len, u8 cmd, s32 addr )
{
#define    MAXDATAFRMLEN6513    115    //090826 96

    s32 i;
    s32 tmp = 0;
    s32 tmplen = 0;
    u8 tmpbuff[ 256 ] = {0};
    u8 databuf[ 256 ] = {0};
    s32 trys = 0;

    s32 outlen = Len;

    for( i = 0; i < Len; )
    {
        tmp = ( outlen > MAXDATAFRMLEN6513 ) ? MAXDATAFRMLEN6513 : outlen;
        
        tmplen = FormSetFrm(  &buff[ i ], tmp, tmpbuff, cmd, addr + i );
        
        s32 outlen1 = sizeof( databuf );
        if( WriteCom( tmpbuff, tmplen, databuf, outlen1, true) )
        {
            s32 val = 0;
            u8 rbuff[ 256 ] = { 0 };
            if( ProcReturnedFrame( databuf, outlen1, addr + i, rbuff, val) == 0  )
            {
                i += tmp;
                outlen -= tmp;
                trys = 0;
                continue;
            }
        }
        
        trys ++;
        if( trys > 4 )
        {
            break;
        }
    }
    printf( "WRITE OVER \n");
    return i;
}
void PrintLog(const char * head,u8 *buf, s32 len)
{
	printf("%s,len=%d:\n",head,len);
	for(s32 i = 0; i < len; i++)
	{
		printf("%02x ",buf[i]);
	}
	printf("\n");
}
bool CProtocol::WriteCom( u8 *buf, s32 Len, u8 *outbuf, s32 &outLen, bool m_flags)
{
    s32 m_result = m_AsyncCom.serial_query( buf, Len, outbuf, outLen, 2000 );
	PrintLog( "S(mp0):", buf,Len);
	
	PrintLog( "R(mp0):", outbuf,outLen);

    if( m_result & ( SERIAL_READ_FAIL | SERIAL_WRITE_FAIL | SERIAL_OPEN_FAIL | SERIAL_CLOSE_FAIL | SERIAL_TIMEOUT ) )
    {
        printf( "WriteCom() err! m_result=%04x\n", m_result );
        return false;
    }
    else 
    {
        return true;
    }
}



s32 CProtocol::FormQueryFrm( u8 *buff, u8 cmd, s32 addr )
{
    s32 i;
    u8 sum = 0;
    u8 *p = buff;
    *p ++ = 0x68;
    for( i = 0; i < 6; i ++)
    {
        *p++ = 0xAA;
    }
    *p++ = 0x68;
    *p++ = cmd;
    *p++ = 2;
    *p++ = (addr & 0xFF) + 0x33;
    *p++ = ((addr >> 8) & 0xFF) + 0x33;
    for( i = 0; i < (p - buff); i ++)
    {
        sum += buff[ i ];
    }
    *p ++ = sum;
    *p ++ = 0x16;
    return (p - buff);
}

s32 CProtocol::FormCheckClock( u8 *buff )
{
    tm pTime;
    time_t now_time = time( NULL );
    localtime_r( &now_time, &pTime );
    buff[0] = 0xfe;
    buff[1] = 0x68;
    for(s32 i = 2;i < 8; i++)
        buff[ i ] = 0x99;       //广播命令地址
    buff[8] = 0x68;
    buff[9] = 0x08;             //控制码    
    buff[10] = 0x06;            //数据长度
    
    buff[11] = _Hex2Bcd( pTime.tm_sec ) + 0x33;//0~59
    buff[12] = _Hex2Bcd( pTime.tm_min ) + 0x33;//0~59
    buff[13] = _Hex2Bcd( pTime.tm_hour) + 0x33;//0~23
    buff[14] = _Hex2Bcd( pTime.tm_mday) + 0x33;//1~31
    buff[15] = _Hex2Bcd( pTime.tm_mon + 1) + 0x33;//0~11 + 1
    buff[16] = _Hex2Bcd( pTime.tm_year - 100) + 0x33;//since 1900 year - 100 = since 2003

    u8 sum = 0;
    for( s32 j = 1; j < 17; j ++ ) sum += buff[ j ];
    buff[17] = sum;
    buff[18] = 0x16;
    buff[19] = 0x00;
    return 20;
}


s32 CProtocol::ProcReturnedFrame( u8 *inbuff, s32 inLen,s32 item, u8 *outbuff,s32 &outlen )
{
    u8 *pData = ProcHeadFrame( inbuff, inLen);
    //查找数据帧的头部信息
    if(pData == NULL)
    {
        return 0x0100;
    } 
        
    memcpy( MeterAddr,  &pData[ 1 ], 6 );
    return ProcFullFrame(pData, item,outbuff, outlen);

}
s32 CProtocol::ProcFullFrame( u8 *data, s32 item,u8 *outbuff, s32 &outlen )
{
    s32  j;
    if( data[ 8 ] & 0x40 ) return 0x800;
    switch(data[ 8 ])
    {
        case 0x81://读数据返回，无后续数据
        case 0xA1://读数据返回，有后续数据
        case 0x82://请求后续数据返回，无后续数据
        case 0xA2://请求后续数据返回，有后续数据
        case 0x83://重读数据，无后续
        case 0xA4://重读数据，有后续
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0xB0:
                for(j = 0; j < data[9]; j ++)
                {
                    data[10 + j] -= 0x33;//数据域-0x33
                }
                if(item != data[ 11 ] * 256 + data[ 10 ])
                {
                    return 0x0500;//抄表项目不一样
                }
                data[9] -=  2;
                outlen = data[9];
                memcpy( (void *)outbuff, (void *)&data[ 12 ], data[9]);
                break;
        case 0xC1://读数据返回异常
        case 0xC2://读后续数据返回异常
        case 0xC3://重读数据返回异常
        case 0xC4://写数据，异常应答
            data[10] -= 0x33;
            if( data[ 10] & 0x03 )    return 0x600;
            return data[ 10 ]; //电表返回错误码
        case 0x84://写数据，从站正常应答
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
            break;
        case 0x8C:
            break;//更改通讯速率，正常应答
        default:
            return 0x0700;//无效的控制字符
    }
    return 0;
}


s32 CProtocol::FormSetFrm(  u8 *buff, s32 len, u8 *obuff, u8 cmd, s32 addr )
{
    s32 i;
    u8 *p = obuff;
    *p ++ = 0x68;
    for( i = 0; i < 6; i ++)
    {
        *p ++ = 0xAA;
    }
    *p ++ = 0x68;
    *p ++ = cmd;
    *p ++ = len + 2;
    *p ++ = (addr& 0xFF) + 0x33; 
    *p ++ = (addr >> 8 ) + 0x33;
    for( i = 0; i < len; i ++ )
    {
        *p++ = buff[ i ] + 0x33;
    }
    u8 sum = 0;
    for( i = 0; i < (p - obuff ); i ++)
    {
        sum += obuff[ i ];
    }
    *p ++ = sum;
    *p ++ = 0x16;
    return (p - obuff);
}

u8* CProtocol::ProcHeadFrame(u8 *inbuff, s32 inLen)
{
    s32 i =0;
    while( i <= (inLen - 10 - 2))////2*0x68 + 6 * adr + 1 * 0x16 + 1 * crc + 1*Len + 1* control
    {
        if(inbuff[i] == 0x68 && inbuff[i + 7] == 0x68)//找到数据帧头部
        {
            break;
        }
        i++;
    }
    if(i > (inLen - 10 - 2))//在有效数据长度内没有找到同步字
    {
        printf( "ProcHeadFrame1\n");
        return NULL;//数据长度不正确
    }
    
    u8 *pData = &inbuff[i];//数据起始
    u16 Len  = pData[9] + 10 + 2; //2* 0x68 + 6 * adr + 1 * 0x16 + 1 * crc + 1*Len + 1* control
    if(Len > inLen - i)
    {
        printf( "ProcHeadFrame2\n");
        return NULL;
    }
    if(pData[Len - 1] != 0x16)
    {
        printf( "ProcHeadFrame3\n");
        return NULL;//没有结束标志
    }
    
    u8 m_check = 0;
    for( s32 k = 0; k < (Len - 2 ); k ++)
    {
        m_check += pData[ k ];
    }
    //= GetCheckSum(pData,Len - 2);
    if(m_check != pData[Len - 2])
    {
        printf( "ProcHeadFrame4\n");
        return NULL; //校验和不对
    }
    return pData;
}
