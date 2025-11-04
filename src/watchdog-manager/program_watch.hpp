#ifndef _PROGRAM_WATCH_HPP
#define _PROGRAM_WATCH_HPP

#include <string>
#include "config_file.hpp"

using namespace std;

class ProgramWatch {
public:
    ProgramWatch();
    ~ProgramWatch();

    void Run();
    void Watch();
    void SendSignal();// now, we only send SIGUSR1 signal instantly after detected poweroff event
    void SetConfigFilePath(string configPath);
    bool IsNeedReboot();

private:
    void RebootSystem();
    bool IsProgramRunning(int pid);
    void StartProgram(Program& program);

    bool m_needReboot;
    string m_configFileName;
    list<struct Program> m_programList;
};

 


#endif // _PROGRAM_WATCH_HPP
