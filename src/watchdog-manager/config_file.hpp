#ifndef _CONFIG_FILE_HPP
#define _CONFIG_FILE_HPP

#include <list>

enum process_mode {
    RERUN = 1,
    REBOOT = 2,
    ONCE = 3
};

struct Program {
    int pid;
    enum process_mode mode;
    char * path;
    char * param[50];
};

class ConfigFile {
public:
    ConfigFile();
    ~ConfigFile();

    void Read(char const * fileName);
    std::list<struct Program> GetPrograms();

private:
    FILE * m_fp;
    std::list<struct Program> m_programs;  

    static const char * defaultConfigFile;

    void OpenFile(char const * fileName);
};

#endif // _CONFIG_FILE_HPP
