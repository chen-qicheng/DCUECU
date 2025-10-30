#include "SerialPort.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <termios.h>

#include <iostream>

#include <poll.h>
#include <signal.h>


// 初始化静态成员
SerialPort SerialPort::instance;

bool SerialPort::open(const char* portname, int baudrate, char parity, char databit, char stopbit, char synchronizeflag)
{
    std::cout << "SerialPort::open" << std::endl;
    // 打开串口
    m_fd = ::open(portname, O_RDWR | O_NOCTTY);
    
    // 打开失败，则打印失败信息，返回false
    if(m_fd == -1)
    {
        std::cout << portname << " open failed , may be you need 'sudo' permission." << std::endl;
        return false;
    }

    // 设置串口参数
    // 创建串口参数对象
    struct termios options;
    bzero(&options, sizeof(options));

    /* 使能接收 */
    options.c_cflag |= CBAUD | CLOCAL | CREAD;

    // 设置波特率
    switch(baudrate)
    {
        case 4800:
            cfsetispeed(&options,B4800);
            cfsetospeed(&options,B4800);
            break;
        case 9600:
            cfsetispeed(&options,B9600);
            cfsetospeed(&options,B9600);
            break;   
        case 19200:
            cfsetispeed(&options,B19200);
            cfsetospeed(&options,B19200);
            break;
        case 38400:
            cfsetispeed(&options,B38400);
            cfsetospeed(&options,B38400);
            break;
        case 57600:
            cfsetispeed(&options,B57600);
            cfsetospeed(&options,B57600);
            break;
        case 115200:
            cfsetispeed(&options,B115200);
            cfsetospeed(&options,B115200);
            break;
        default:
            std::cout << portname << " open failed , unkown baudrate , only support 4800,9600,19200,38400,57600,115200." << std::endl;
            return false;
    }

    // 设置校验位
    switch(parity)
    {
        // 无校验
        case 0:
            options.c_cflag &= ~PARENB;//PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag &= ~INPCK;//INPCK：使奇偶校验起作用
            break;
        // 设置奇校验
        case 1:
            options.c_cflag |= PARENB;//PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag |= PARODD;//PARODD：若设置则为奇校验,否则为偶校验
            options.c_cflag |= INPCK;//INPCK：使奇偶校验起作用
            options.c_cflag |= ISTRIP;//ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
            break;
        // 设置偶校验
        case 2:
            options.c_cflag |= PARENB;//PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag &= ~PARODD;//PARODD：若设置则为奇校验,否则为偶校验
            options.c_cflag |= INPCK;//INPCK：使奇偶校验起作用
            options.c_cflag |= ISTRIP;//ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
            break;
        default:
            std::cout << portname << " open failed , unkown parity ." << std::endl;
            return false;
    }

    // 设置数据位
    switch(databit)
    {
        case 5:
            options.c_cflag &= ~CSIZE;//屏蔽其它标志位
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag &= ~CSIZE;//屏蔽其它标志位
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag &= ~CSIZE;//屏蔽其它标志位
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag &= ~CSIZE;//屏蔽其它标志位
            options.c_cflag |= CS8;
            break;
        default:
            std::cout << portname << " open failed , unkown databit ." << std::endl;
            return false;
    }

    // 设置停止位
    switch(stopbit)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;//CSTOPB：使用1位停止位
            break;
        case 2:
            options.c_cflag |= CSTOPB;//CSTOPB：使用2位停止位
            break;
        default:
            std::cout << portname << " open failed , unkown stopbit ." << std::endl;
            return false;
    }

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST; 
    options.c_cflag &= ~CRTSCTS;
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* 将MIN和TIME设置为0 */
    options.c_cc[VTIME] = 4;
    options.c_cc[VMIN] = 255;

    tcflush(m_fd, TCIFLUSH);

    // 激活新配置
    if((tcsetattr(m_fd,TCSANOW,&options))!=0) 
    { 
        std::cout << portname << " open failed , can not complete set attributes ." << std::endl;
        return false; 
    } 

    int flags=0;
    if( (flags=fcntl(m_fd, F_GETFL))==-1 ||
        fcntl(m_fd, F_SETFL, flags&~O_NDELAY)==-1 )
    {
        printf("fcntl error!\n");
        ::close(m_fd);
        m_fd = -1;
        return -2;
    }
    tcflush(m_fd, TCIFLUSH);
    tcflush(m_fd, TCOFLUSH);  
        
    return true;
}



void SerialPort::close()
{
    if(m_fd != -1)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

int SerialPort::send(const uint8_t *buf,int len)
{
    return write(m_fd, buf, len);
    // int sendCount = 0;
    // if(m_fd != -1)
    // {   
    //     // 将 buf 和 len 转换成api要求的格式
    //     const char *buffer = (char*)buf;
    //     size_t length = len;
    //     // 已写入的数据个数
    //     ssize_t tmp;

    //     while(length > 0)
    //     {
    //         if((tmp = ::write(m_fd, buffer, length)) <= 0)
    //         {
    //             if(tmp < 0&&errno == EINTR)
    //             {
    //                 tmp = 0;
    //             }
    //             else
    //             {
    //                 break;
    //             }
    //         }
    //         length -= tmp;
    //         buffer += tmp;
    //     }

    //     sendCount = len - length;
    // }
   
    // return sendCount;
}

int SerialPort::receive(uint8_t *buf,int maxlen)
{
    int r_len=0;
    pollfd pd;
    pd.fd = m_fd;
    pd.events = POLLIN | POLLPRI;
    sigset_t intmask;
    sigfillset(&intmask);
    sigprocmask(SIG_BLOCK, &intmask, NULL);

    int ret = poll(&pd, 1, 500);

    // printf("[%s][%d] poll ret = %d\n",__FUNCTION__, __LINE__, ret);
    if(ret > 0)
    {
        sigprocmask(SIG_UNBLOCK, &intmask, NULL);
        r_len =  ::read(m_fd,buf,maxlen);	
        return r_len;
    }
    else if(ret == 0)
    {
        sigprocmask(SIG_UNBLOCK, &intmask, NULL);
        return -1;
    }
    else
    {
        sigprocmask(SIG_UNBLOCK, &intmask, NULL);
        return -1;
    }
    return -1;

    // int receiveCount = ::read(m_fd,buf,maxlen);
    // if(receiveCount < 0)
    // {
    //     receiveCount = 0;
    // }
    // return receiveCount;
}
