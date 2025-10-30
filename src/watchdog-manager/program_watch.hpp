#ifndef _PROGRAM_WATCH_HPP
#define _PROGRAM_WATCH_HPP

#include "config_file.hpp"

class ProgramWatch {
public:
    ProgramWatch();
    ~ProgramWatch();

    void Run();
    void Watch();
    void SendSignal();// now, we only send SIGUSR1 signal instantly after detected poweroff event
    void SetConfName( char * name );
    void Init();

    bool isReboot;
    
private:
    void RebootSystem();
    class ConfigFile ProgConf;
    char ProgConfName[150];

    std::list<struct Program> m_ProgramList;
};

 


#endif // _PROGRAM_WATCH_HPP
