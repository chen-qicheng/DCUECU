#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "watchdog.hpp"

const string WATCHDOG_FILE = "/dev/wfet1000wdog";

Watchdog::Watchdog()
{
    m_fd = -1;
}


// to open Watchdog device and to active watchdog device 
bool Watchdog::Open()
{
    string file = WATCHDOG_FILE;

    m_fd = open(file.c_str(), O_RDWR); 

    if (m_fd < 0) 
    {
        m_fd = -1;
        return false;
    }
    return true;
}


bool Watchdog::Close()
{
    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;
        return false;
    }
    return true;
}


// to "feed watchdog" in case of system reboot
bool Watchdog::Feed()
{
    unsigned char commandBuff[2];

    commandBuff[0] = 0x00;
    commandBuff[1] = 0x80;

    int result = write(m_fd, commandBuff, 2);

    if(result != 2) 
    {
        return false;
    }
    return true;
}