#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include    "AsyncCom.h"
//#include "SysDef.h"

#define u8  unsigned char
#define s8  char
#define s32  int
#define u32 unsigned int
#define u16 unsigned short
#define s16 short

class CProtocol
{
public: 
    CProtocol( );
    virtual ~CProtocol( );
public:
    void    Init( u8 serial_num, s32 buad );
    s32     Get( u8 *buff, s32 Len, u8 cmd, s32 addr );
    s32     Set( u8 *buff, s32 Len, u8 cmd, s32 addr );
    void    SetClock( );
public:
    //char    MeterAddr[6];
private:
    s32     FormCheckClock( u8 *buff );
    bool    WriteCom( u8 *buf, s32 Len, u8 *outbuf, s32 &outLen, bool m_flags);
    s32     FormQueryFrm( u8 *buff, u8 cmd, s32 addr );
    s32     ProcReturnedFrame( u8 *inbuff, s32 inLen,s32 item, u8 *outbuff,s32 &outlen );
    s32     ProcFullFrame(u8 *data, s32 item,u8 *outbuff, s32 &outlen);
    s32     FormSetFrm(  u8 *buff, s32 len, u8 *obuff, u8 cmd, s32 addr );
    u8 *ProcHeadFrame(u8 *inbuff, s32 inLen);
private:
    CAsyncCom m_AsyncCom;

};
#endif
