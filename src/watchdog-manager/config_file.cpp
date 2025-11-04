#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "config_file.hpp"

const string DEFAULT_CONFIG_FILE = "/home/et1000/conf/watchdog.conf";

ConfigFile::ConfigFile()
{
    m_programList.clear();
}

ConfigFile::~ConfigFile()
{
    m_programList.clear();
}

bool isDelimiter(char c) 
{
    return c == ' ' || c == '=' || c == ',';
}

std::vector<std::string> splitString(const std::string& str) 
{
    std::vector<std::string> result;
    std::string currentSubstr;
    
    for (char c : str) 
    {
        // 如果是分隔符
        if (isDelimiter(c)) 
        {
            // 如果当前有积累的子串，添加到结果中
            if (!currentSubstr.empty()) 
            {
                result.push_back(currentSubstr);
                currentSubstr.clear();
            }
        } 
        else 
        {
            // 不是分隔符则添加到当前子串
            currentSubstr += c;
        }
    }
    
    // 处理最后一个子串
    if (!currentSubstr.empty()) 
    {
        result.push_back(currentSubstr);
    }
    
    return result;
}


void ConfigFile::Read(string fileName) 
{
    std::ifstream configFile(fileName);
    if (!configFile.is_open()) 
    {
        return;
    }

    std::string line;
    while (std::getline(configFile, line)) 
    {
        struct Program tempNode;
        tempNode.pid = -1;
        tempNode.param_count = 0;

        std::vector<std::string> tokens = splitString(line);
        if (tokens.size() < 2) 
        {
            continue;
        }

        if (tokens[0] == "reboot")
        {
            tempNode.mode = REBOOT;
        }
        else if (tokens[0] == "rerun")
        {
            tempNode.mode = RERUN;
        }
        else if (tokens[0] == "once")
        {
            tempNode.mode = ONCE;
        }
        else 
        {
            continue;
        }

        tempNode.path = tokens[1];
        
        for (size_t i = 2; i < tokens.size() && tempNode.param_count < 50; ++i)
        {
            tempNode.param[tempNode.param_count++] = tokens[i];
        }

        m_programList.push_back(tempNode);
    }
    return;
}


std::list<struct Program> ConfigFile::GetPrograms()
{
    return m_programList;
}