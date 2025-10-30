#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "config_file.hpp"

const char * ConfigFile::defaultConfigFile = "/home/et1000/conf/watchdog.conf";

ConfigFile::ConfigFile()
{
    m_programs.clear();
    m_fp = nullptr;
}

ConfigFile::~ConfigFile()
{
    if (m_fp != nullptr) 
    {
        fclose(m_fp);
        m_fp = nullptr;
    }

    m_programs.clear();
}

void ConfigFile::OpenFile(char const * fileName)
{
    // 如果已经有打开的文件，先关闭它避免资源泄漏
    if (m_fp != nullptr) 
    {
        fclose(m_fp);
        m_fp = nullptr;
    }

    if (fileName == nullptr  || strlen(fileName) == 0)
    {
        if (access(defaultConfigFile, R_OK) == 0 )
        {
            fileName = defaultConfigFile;
        }
    }

    m_fp = fopen(fileName, "r");
    if (m_fp == 0) 
    {
        fprintf(stderr, "can not open watchdog config file: %s", fileName);
        return;
    }
}


void ConfigFile::Read(char const * fileName) 
{
    char line[160];
    char delimiter[] = " =,\t\n";
    char *token;

    OpenFile(fileName);
    if (m_fp == nullptr) 
    {
        return;
    }

    fseek( m_fp, 0L, SEEK_SET );

    while (fgets(line, sizeof(line), m_fp) != nullptr) 
    {
        struct Program tmpNode;

        char *last;
        token = strtok_r(line, delimiter, &last);

        /* to set program run parameters */
        if ( strcmp( token, "reboot" ) == 0 ) 
        {
            tmpNode.mode = REBOOT;
        }
        else if ( strcmp( token, "rerun" ) == 0 )
        {
            tmpNode.mode = RERUN;
        }
        else if ( strcmp( token, "once" ) == 0 )
        {
            tmpNode.mode = ONCE;
        }
        else 
        {
            continue;
        }

        token = strtok_r(nullptr, delimiter, &last);
        if ( token == nullptr ) 
        {
            continue;
        }
        tmpNode.path = strdup(token);

        for (int i = 0; i < (int)(sizeof(tmpNode.param)/sizeof(char *)); i ++ ) 
        {
            token = strtok_r(nullptr, delimiter, &last);
            if ( token == nullptr ) 
            {
                tmpNode.param[i] = 0;
                break;
            }
            tmpNode.param[i] = strdup(token);
        }

        tmpNode.pid = -1;

        m_programs.push_back(tmpNode);
    }
    return;
}


std::list<struct Program> ConfigFile::GetPrograms()
{
    return m_programs;
}