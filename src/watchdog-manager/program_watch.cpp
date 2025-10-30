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
    isreboot = false;
}

void ProgramWatch::Run()
{
    struct Program * tmp;

    tmp = ProgConf.FirstProg();

    while( tmp != 0 ) 
    {
        tmp->pid = fork();
        if ( tmp->pid < 0 ) 
        {
            exit( -1 );
        }
        else if ( tmp->pid == 0 ) 
        {
            execv( tmp->path, tmp->param );
            exit( -1 );
        }

        tmp = ProgConf.NextProg();
    }
}

void ProgramWatch::Watch()
{
    struct Program * tmp;
    
    tmp = ProgConf.FirstProg();
    while ( tmp != 0 ) 
    {
        if ( waitpid( tmp->pid, 0, WNOHANG ) != 0 ) 
        {
            if ( tmp->mode == REBOOT ) 
            {
                RebootSystem();
                return;
            }
            else if ( tmp->mode == RERUN )
            {
                tmp->pid = fork();
                if ( tmp->pid < 0 ) 
                {
                    RebootSystem();
                }
                else if ( tmp->pid == 0 ) 
                {
                    execv( tmp->path, tmp->param );
                    exit( -1 );
                }
            }
            else
            {   // tmp->pid == ONCE 
                /* do something */
            }
        }
        tmp = ProgConf.NextProg();
    }
}

void ProgramWatch::SendSignal()
{
    struct Program * tmp;
    
    tmp = ProgConf.FirstProg();
    while ( tmp != 0 ) 
    {
        if ( waitpid( tmp->pid, 0, WNOHANG ) == 0 ) 
        {
            kill( tmp->pid, SIGUSR1 );
        }
        tmp = ProgConf.NextProg();
    }
}

void ProgramWatch::SetConfName( char * name )
{
    if ( name == 0 ) 
    {
        return;
    }

    strncpy( ProgConfName, name, sizeof( ProgConfName ) );

    ProgConfName[149] = '\0';
}

void ProgramWatch::Init()
{
    ProgConf.Clear();
    ProgConf.Read( ProgConfName );
}

ProgramWatch::~ProgramWatch()
{
    ProgConf.Clear();
}

void ProgramWatch::RebootSystem()
{
    struct Program * tmp;

    tmp = ProgConf.FirstProg();
    while ( tmp != 0 ) 
    {
        if ( waitpid( tmp->pid, 0, WNOHANG ) == 0 ) 
        {
            kill( tmp->pid, SIGTERM );
        }
        tmp = ProgConf.NextProg();
    }

    isreboot = true;
}
