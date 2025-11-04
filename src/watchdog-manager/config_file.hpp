#ifndef _CONFIG_FILE_HPP
#define _CONFIG_FILE_HPP

#include <list>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

enum process_mode {
    RERUN = 1,
    REBOOT = 2,
    ONCE = 3
};

struct Program {
    int pid;
    enum process_mode mode;
    string path;
    string param[50];
    int param_count;
};

class ConfigFile {
public:
    ConfigFile();
    ~ConfigFile();

    void Read(std::string fileName);
    std::list<struct Program> GetPrograms();

private:
    std::list<struct Program> m_programList;  
};

#endif // _CONFIG_FILE_HPP
