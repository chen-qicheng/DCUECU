#ifndef _CONFIG_FILE_HPP
#define _CONFIG_FILE_HPP

enum process_mode {
    RERUN = 1,
    REBOOT = 2,
    ONCE	= 3
};

struct Program {
    int pid;
    enum process_mode mode;
    char * path;
    char * param[50];
};

struct ProgramList {
    struct Program node;
    struct ProgramList * next;
};


class ConfigFile {
public:
    ConfigFile();
    ~ConfigFile();

    void Read( char const * FileName );
    struct Program *  FirstProg( );
    struct Program *  NextProg( );
    struct Program * CurrProg( );
    void Clear();


private:

    static const char * DefaultConfName;

    struct ProgramList * PrgList;
    struct ProgramList * CurrNode;

};

extern unsigned int rebootinterval;
extern char g_locale_str[20];
extern char g_cputype_str[20];
extern char g_termtype_str[20];
#endif // _CONFIG_FILE_HPP
