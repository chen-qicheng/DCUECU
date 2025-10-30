#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#include <stdint.h>
#include <signal.h>

// 具体产品 - 标准串口
class SerialPort 
{
public:
	// 公有静态方法，提供全局访问点
    static SerialPort& getInstance() 
	{
        return instance;
    }

	// 打开串口,成功返回true，失败返回false
	// portname(串口名): 在Windows下是"COM1""COM2"等，在Linux下是"/dev/ttyS1"等
	// baudrate(波特率): 9600、19200、38400、43000、56000、57600、115200 
	// parity(校验位): 0为无校验，1为奇校验，2为偶校验，3为标记校验（仅适用于windows)
	// databit(数据位): 4-8(windows),5-8(linux)，通常为8位
	// stopbit(停止位): 1为1位停止位，2为2位停止位,3为1.5位停止位
	// synchronizeflag(同步、异步,仅适用与windows): 0为异步，1为同步
	bool open(const char* portname, int baudrate, char parity, char databit, char stopbit, char synchronizeflag=1);

	//关闭串口，参数待定
	void close();

	//发送数据或写数据，成功返回发送数据长度，失败返回0
	int send(const uint8_t *buf,int len);

	//接受数据或读数据，成功返回读取实际数据的长度，失败返回0
	int receive(uint8_t *buf,int maxlen);

private:
	// 私有静态实例，类加载时初始化
	static SerialPort instance;

	// 私有构造函数，防止外部实例化
	SerialPort() = default;

	// 禁用拷贝构造和赋值操作
	SerialPort(const SerialPort&) = delete;
	SerialPort& operator=(const SerialPort&) = delete;
	
	int m_fd = -1;

};


#endif // !__SERIALPORT_H__