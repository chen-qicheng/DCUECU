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
#include <getopt.h>
#include <ftw.h>
#include <log.h>
#include <syslog.h>
#include <ctype.h>

#include "SoftDog.hpp"
#include "program_watch.hpp"
#include "watchdog.hpp"

#include "event_id.h"

#define MAX_FILE_NUM (100)
#define BUFLEN 64
#define LINUM 2
#define MAX_TRIES 3


void InitDaemon(void);
void initenv();

enum PROCESS_STATUS
{
    NO_EXIST,
    RUNNING,
    SLEEPING,
    DISK_SLEEP,
    STOPPED,
    TRACING_STOP,
    ZOMBIE,
    DEAD,
    OTHER,
};


///////////////////////////////////////////////////////////
// ���ܣ���������Ľ��̺Ż�ȡ�ý��̶�Ӧ�Ľ���״̬
////////////////////////////////////////////////////////////
enum PROCESS_STATUS GetPidState( int pid )
{
    char buffer[BUFLEN];
    char state[2] = {0, 0};

    if (snprintf(buffer, BUFLEN, "/proc/%d/status", pid) >= BUFLEN)
    {
        printf("buffer overflow detected\n");
        return NO_EXIST;
    }

    FILE *proc_fs_p;
    proc_fs_p = fopen(buffer , "r");
    if (!proc_fs_p)
    {
        printf("/proc open failed\n");
        return NO_EXIST;
    }

    int coln_count = 1, tmp = 0;

    while (fgets( buffer, BUFLEN, proc_fs_p ) != NULL)
    {
        tmp++;
        if (tmp != LINUM)
        {
            continue;
        }

        for (int i = 0; i < BUFLEN - 1; i++)
        {
            if (buffer[i] == ':')
            {
                coln_count--;
            }
            else if (!coln_count && !isblank(buffer[i]))
            {
                state[0] = buffer[i];
                state[1] = buffer[i + 1];
                break;
            }
        }
        break;
    }
    fclose(proc_fs_p);

    switch (state[0])
    {
        case 'R': return RUNNING;
        case 'W': return OTHER;
        case 'S': return SLEEPING;
        case 'T': return TRACING_STOP;
        case 'Z': return ZOMBIE;
        case 'D': return DISK_SLEEP;
        default:
              printf("unable to determine process state\n");
              return OTHER;
    }

    return OTHER; // unreachable
}

//////////////////////////////////////////////////////////////////
// ���ܣ� ��������롰D��״̬��disk sleep������������ܱ�ɱ������ϵͳ���ܲ���ɥʧ
//         ��ʱ��Ҫ��λϵͳ�Ա�֤ϵͳ����������
// ע�⣺���̴���"D"״̬���ܿ����������ֳ�����������ɵģ���˾�ڲ���EMC���ܲ����ǣ���һ�����ʳ��֣�
//       ����������������Ҫ��λ����
//       �жϳ����ܷ�ɱ��
///////////////////////////////////////////////////////////////////
void SaveSystem( int pid )
{
    for (int i = 0; i < MAX_TRIES; i ++)
    {
        kill( pid, SIGKILL );
        sleep(1);
        if (GetPidState ( pid ) != DISK_SLEEP)
        {
            return;
        }
    }
    syslog( LOG_CONS|LOG_WARNING, "����ɱ�����̣�ϵͳ��λ.\n" );
	
    system( "/sbin/reboot" );
    sleep( 20 );
    system( "/usr/bin/killall -9 watchdog" );
}

int Watch(const char *file, const struct stat *sb, int flag)
{
    struct process_t process = {0, 1, 0, ""};

    if (flag == FTW_F)
    {
        FILE* fp = fopen(file, "rb");
        if(fp == NULL)
        {
            return 0;
        }
        fread(&process, sizeof(process_t), 1, fp);
        fclose( fp );

        struct sysinfo info ;
        if (sysinfo( &info ) < 0)
        {
            return 0;
        }

        if (process.up_time + process.dead_seconds < info.uptime)
        {
            if (process.pid > 0)
            {
                if ( GetPidState( process.pid ) == DISK_SLEEP )
                {
                    syslog(LOG_CONS|LOG_WARNING, "����%s�쳣�����̴���'D'״̬.\n", process.name);
                    SaveSystem(process.pid);
                }
                kill(process.pid, SIGKILL);
                syslog(LOG_CONS|LOG_WARNING, "����%s�쳣�����Ź����临λ\n", process.name);
            }
            remove(file);
        }
    }
    return 0;
}


int main(int argc, char *argv[], char *env[])
{

    //������Ź�����Ŀ¼
    char command [sizeof(SOFT_WATCH_PATH) + 10];
    sprintf(command, "rm -rf %s", SOFT_WATCH_PATH);
    system(command);

    mkdir(SOFT_WATCH_PATH, S_IRWXU);

    class ProgramWatch progs;
    class Watchdog watchdog;

    progs.SetConfName(configfile);

    // to mask the SIGTERM signal
    signal( SIGTERM, SIG_IGN );

    progs.Init();

    InitDaemon();

    initenv();

    watchdog.Open();
    watchdog.Feed();
    watchdog.Feed();
    watchdog.Feed();

    progs.Run();

    
    while (true) 
    {
        sleep(1);
        progs.Watch();

        if (progs.isReboot) 
        {
            perror("program crash, system must be reboot!\n");
            // to reboot system
            for(int i = 0; i < 30; i ++) 
            {
                watchdog.Feed();
                sleep(1);
            }
	  
            system("/sbin/reboot");
        }
        watchdog.Feed();
        ftw(SOFT_WATCH_PATH, Watch, MAX_FILE_NUM);
    }

    return 0;
}

void InitDaemon(void)
{
    struct sigaction act;
    int max_fd, i, ret;

    /*���е�1��fork,Ϊsetsid��׼�� */
    ret = fork();

    if (ret < 0) 
    {
        perror("InitDaemon() fork failed!");
        exit(1);
    }
    else if (ret != 0)
    {
        exit(0);
    }

    /*����setsid��ʹ�ý��̾ɻỰ���̷��� */
    ret = setsid();
    if (ret < 0) 
    {
        perror("InitDaemon() setsid failed!");
        exit(1);
    }

    /*�����ź�SIGHUP */
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGHUP, &act, 0);

    /*
    *���е�2��fork��ʹ���̲����ǻỰ���̵���ͷ���̣���������ٴ�
    *�ն���Ϊ�Լ��Ŀ����ն�
    */
    ret = fork();

    if (ret < 0) 
    {
        perror("InitDaemon() fork failed!");
        exit(1);
    }
    else if (ret != 0)
    {
        exit(0);
    }

    /*�޸Ľ��̵ĵ�ǰĿ¼ */
    chdir("/");

    /*������̵��ļ����� */
    umask(0);

    /*ʹ�ý�������ԭ���Ľ����飬�����ܷ���ԭ����������źŵĸ��� */
    setpgrp();

    /*�رս������е��ļ������� */
    max_fd = sysconf(_SC_OPEN_MAX);
    for (i = 0; i < max_fd; i++)
    {
        close(i);
    }

    /*�򿪿��豸��ʹ�ú����������䲻������� */
    if ((i = open("/dev/null", O_RDWR)) >= 0) 
    {
        while (0 <= i && i <= 2)
        {
            i = dup(i);
        }
            
        if (i >= 0)
        {
            close(i);
        }  
    }
    return;
}


void initenv()
{
    setenv( "HOME", "/home/et1000", 1 );
    chdir( "/home/et1000" );
}

