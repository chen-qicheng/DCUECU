#ifndef _WATCHDOG_HPP
#define _WATCHDOG_HPP

#include <string>
using namespace std;

/////////////////////////////////////////////////////////////
// to manage watchdog device
////////////////////////////////////////////////////////////
class Watchdog {
public:
    Watchdog();
    bool Feed(); //to reset watchdog device and prevent system reboot
    bool Open();
    bool Close();

private:
    int m_fd;
};

#endif //_WATCHDOG_HPP
