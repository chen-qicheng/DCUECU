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
// 功能：根据输入的进程号获取该进程对应的进程状态
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
// 功能： 当程序进入“D”状态（disk sleep）后，如果程序不能被杀死，则系统功能部分丧失
//         这时需要复位系统以保证系统功能正常。
// 注意：进程处于"D"状态，很可能是由于现场环境干扰造成的，公司内部做EMC性能测试是，有一定概率出现，
//       类似于死机现象，需要复位处理
//       判断程序能否杀死
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
    syslog( LOG_CONS|LOG_WARNING, "不能杀死进程，系统复位.\n" );
	
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
                    syslog(LOG_CONS|LOG_WARNING, "进程%s异常，进程处于'D'状态.\n", process.name);
                    SaveSystem(process.pid);
                }
                kill(process.pid, SIGKILL);
                syslog(LOG_CONS|LOG_WARNING, "进程%s异常，软看门狗将其复位\n", process.name);
            }
            remove(file);
        }
    }
    return 0;
}


int main(int argc, char *argv[], char *env[])
{

    //清空软看门狗工作目录
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

    /*进行第1次fork,为setsid作准备 */
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

    /*调用setsid，使得进程旧会话过程分离 */
    ret = setsid();
    if (ret < 0) 
    {
        perror("InitDaemon() setsid failed!");
        exit(1);
    }

    /*忽略信号SIGHUP */
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGHUP, &act, 0);

    /*
    *进行第2次fork，使进程不再是会话过程的领头进程，因而不能再打开
    *终端作为自己的控制终端
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

    /*修改进程的当前目录 */
    chdir("/");

    /*清除进程的文件掩码 */
    umask(0);

    /*使得进程脱离原来的进程组，不再受发往原来进程组的信号的干扰 */
    setpgrp();

    /*关闭进程所有的文件描述符 */
    max_fd = sysconf(_SC_OPEN_MAX);
    for (i = 0; i < max_fd; i++)
    {
        close(i);
    }

    /*打开空设备，使得后续的输出语句不会出问题 */
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

