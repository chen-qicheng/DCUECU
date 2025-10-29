




int main()
{ 
    /* 创建Service和gprs进程之间通信消息队列 */
    g_SVCMsgQueIdUpLinkSend = msgget(MSG_SERVICE_TO_GPRS, 0666 | IPC_CREAT);
    if (-1 == g_SVCMsgQueIdUpLinkSend)
    {
        SVC_LOG(DEBUG_K, "Create send message queue failed.\n");
        return SVC_FAIL;
    }
    g_SVCMsgQueIdUpLinkRcv = msgget(MSG_GPRS_TO_SERVICE, 0666 | IPC_CREAT);
    if (-1 == g_SVCMsgQueIdUpLinkRcv)
    {
        SVC_LOG(DEBUG_K, "Create receive message queue failed.\n");
        return SVC_FAIL;
    }

}



void SVC_RecvUpLinkMsgQue()
{
    static unsigned char *pCombineDataBuff = NULL;
    static unsigned short curCombineDataLen = 0;

    my_msgbuf msg_buf = {0};

    int ret = msgrcv( g_SVCMsgQueIdUpLinkRcv, &msg_buf, M_MSG_LEN, 0, IPC_NOWAIT );
    if (ret <= 0)
    {
        return;
    }

    unsigned short frameLen =(unsigned short) ret;
    unsigned char *pFrameBuff = msg_buf.pbuf;

    SVC_LogVerboseBuff("recv from uplink:",pFrameBuff,frameLen);


    SVC_LOG(DEBUG_K,"DLMS Ip data...\n");
    SVC_ProcessUplinkPkt(pFrameBuff,frameLen);


}

int SVC_DlmsServerinit()
{
    int ret = 0;
    int port = 0;//tcp 连接不在此service进程建立，无需tcp端口
    GX_TRACE_LEVEL trace = GX_TRACE_LEVEL_VERBOSE;

    CGXDLMSAssociationLogicalName *pLN = new CGXDLMSAssociationLogicalName();
    CGXDLMSTcpUdpSetup *pWrapper = new CGXDLMSTcpUdpSetup();


    if(NULL == pLN|| NULL == pWrapper)
    {
        SVC_LOG(DEBUG_K, "create object fail,%s,line %d.\n", __FILE__, __LINE__);
        return SVC_FAIL;
    }
    //AARQ设置10分钟的超时时间
    pWrapper->SetInactivityTimeout(600);

    g_pSVCLn47Sever = new CGXDLMSServerLN_47(pLN,pWrapper);

    if(NULL == g_pSVCLn47Sever)
    {
        SVC_LOG(DEBUG_K, "create object fail,%s,line %d.\n", __FILE__, __LINE__);
        return SVC_FAIL;
    }

    ret = g_pSVCLn47Sever->Init(port + 3, trace);
    if (ret != 0)
    {
        SVC_LOG(DEBUG_K, "dlms 47 server init fail.\n");
        return SVC_FAIL;
    }

    return SVC_SUCCSS;
}


void SVC_ProcessUplinkPkt(unsigned char *buff, unsigned short len)
{
    CGXByteBuffer reply;
    if(DLMS_ERROR_CODE_OK == g_pSVCLn47Sever->HandleRequest(buff,len,reply))
    {
        SVC_SendPkt2UpLink(reply.GetData(),reply.GetSize());
    }
}


void SVC_SendPkt2UpLink( unsigned char *buff, unsigned int len)
{
    my_msgbuf msg_buf = {0};
    //此处消息类型需为1，不然gprs进程收不到
    msg_buf.msgtype = 1;
    if(len > M_MSG_LEN)
    {
        SVC_LOG(DEBUG_K, "SVC_SendPkt2UpLink,too long buff\n");
        return;
    }
    memcpy(msg_buf.pbuf,buff,len);
    //发送的内容必须是msg_buf结构,但长度是msg_buf.pbuf装载内容的长度
    int ret = msgsnd(g_SVCMsgQueIdUpLinkSend, &msg_buf, len,IPC_NOWAIT);
    if (ret < 0)
    {
        SVC_LOG(DEBUG_K, "svc send to uplink failed,ret = %d.\n", ret);
        return;
    }
    SVC_LogVerboseBuff("send to uplink :",buff,len);

}