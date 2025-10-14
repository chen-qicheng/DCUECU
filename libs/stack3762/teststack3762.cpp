#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include <stdlib.h>
#include "stack3762.h"

using namespace std;
using  std::string;


int Logbuf(unsigned char *pinbuff,const int buflen)
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

int LOGchar(unsigned char *outFrame,unsigned int outFrameLen)
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

int main(int argc,char *argv[])
{
    //unsigned char testBuf[512]={0};
    //unsigned short testBufLen = 0;
    cGDW3762_FARME GDW3762Obj;
    unsigned char dcAddr[GDW3762_MAC_ADDR_LEN]={0x01,0x02,0x03,0x04,0x05,0x06};
        
    //查询主节点地址（AFN=03H-F4）
    GDW3762Obj.FormAfn03(4);
    printf("AFN=03H-F4\n");
    Logbuf(GDW3762Obj.sendFarme.data, GDW3762Obj.sendFarme.dataLen);

    //设置主节点地址（AFN=05H-F1）
    GDW3762Obj.FormAfn05F1(dcAddr);
    printf("AFN=05H-F1\n");
    Logbuf(GDW3762Obj.sendFarme.data, GDW3762Obj.sendFarme.dataLen);

    //暂停路由（AFN=12H-F2）
    GDW3762Obj.FormAfn12(2);
    printf("AFN=12H-F2\n");
    Logbuf(GDW3762Obj.sendFarme.data, GDW3762Obj.sendFarme.dataLen);

    //启用路由（AFN=12H-F3）
    GDW3762Obj.FormAfn12(3);
    printf("AFN=12H-F3\n");
    Logbuf(GDW3762Obj.sendFarme.data, GDW3762Obj.sendFarme.dataLen);
    
    return 0;
}

