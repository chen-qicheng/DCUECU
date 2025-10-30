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
    ProgConfName[0] = '\0';
    isReboot = false;
    Init();
}

ProgramWatch::~ProgramWatch()
{
}

void ProgramWatch::Init()
{
    ProgConf.Read(ProgConfName);
    m_ProgramList = ProgConf.GetPrograms();
}

void ProgramWatch::Run()
{
    for(auto& program : m_ProgramList)
    {
        program.pid = fork();
        if (program.pid < 0 ) 
        {
            exit(-1);
        }
        else if (program.pid == 0) 
        {
            execv(program.path, program.param);
            exit(-1);
        }
    }
}

void ProgramWatch::Watch()
{
    for(auto& program : m_ProgramList)
    {
        if (waitpid(program.pid, 0, WNOHANG) != 0) 
        {
            if (program.mode == REBOOT) 
            {
                RebootSystem();
                return;
            }
            else if (program.mode == RERUN)
            {
                program.pid = fork();
                if (program.pid < 0) 
                {
                    RebootSystem();
                }
                else if (program.pid == 0) 
                {
                    execv(program.path, program.param);
                    exit(-1);
                }
            }
            else
            {   // program.pid == ONCE 
                /* do something */
            }
        }
    }
}

void ProgramWatch::SendSignal()
{
    for(auto& program : m_ProgramList)
    {
        if (waitpid(program.pid, 0, WNOHANG) == 0) 
        {
            kill(program.pid, SIGUSR1);
        }
    }
}

void ProgramWatch::SetConfName(char * name)
{
    if (name == nullptr) 
    {
        return;
    }

    strncpy(ProgConfName, name, sizeof(ProgConfName));

    ProgConfName[149] = '\0';
}


void ProgramWatch::RebootSystem()
{
    for(auto& program : m_ProgramList)
    {
        if (waitpid(program.pid, 0, WNOHANG ) == 0) 
        {
            kill(program.pid, SIGTERM);
        }
    }

    isReboot = true;
}
