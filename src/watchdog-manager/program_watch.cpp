#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


#include "program_watch.hpp"

ProgramWatch::ProgramWatch()
{
    ConfigFile configReader;
    configReader.Read(m_configFileName);
    m_programList = configReader.GetPrograms();

    m_needReboot = false;
}

ProgramWatch::~ProgramWatch()
{
}

void ProgramWatch::Run()
{
    for(auto& program : m_programList)
    {
        StartProgram(program);
    }
}


void ProgramWatch::StartProgram(Program& program)
{
    program.pid = fork();
    if (program.pid < 0) 
    {
        exit(-1);
    }
    else if (program.pid == 0) 
    {
        std::vector<char*> args;

        for (int i = 0; i < program.param_count; ++i) 
        {
            args.push_back(const_cast<char*>(program.param[i].c_str()));
        }
        args.push_back(nullptr);

        execv(program.path.c_str(), args.data());
        
        exit(-1);
    }
}


bool ProgramWatch::IsProgramRunning(int pid)
{
    return waitpid(pid, 0, WNOHANG) == 0;
}

void ProgramWatch::Watch()
{
    for(auto& program : m_programList)
    {
        if (!IsProgramRunning(program.pid)) 
        {
            if (program.mode == REBOOT) 
            {
                RebootSystem();
                return;
            }
            else if (program.mode == RERUN)
            {
                StartProgram(program);
            }
            else if (program.mode == ONCE)
            {   
                // do nothing
            }
        }
    }
}

void ProgramWatch::SendSignal()
{
    for(auto& program : m_programList)
    {
        if (IsProgramRunning(program.pid)) 
        {
            kill(program.pid, SIGUSR1);
        }
    }
}

void ProgramWatch::SetConfigFilePath(string configPath)
{
    m_configFileName = configPath;
}


void ProgramWatch::RebootSystem()
{
    for(auto& program : m_programList)
    {
        if (IsProgramRunning(program.pid))
        {
            kill(program.pid, SIGTERM);
        }
    }

    m_needReboot = true;
}


bool ProgramWatch::IsNeedReboot()
{ 
    return m_needReboot; 
}