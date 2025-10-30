#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "config_file.hpp"

const char * ConfigFile::DefaultConfName = "/home/et1000/conf/watchdog.conf";

unsigned int rebootinterval;

// 默认的cpu 类型为2410
char g_cputype_str[20] = "2410";

// 默认的规约地方版本为 Standard
char g_locale_str[20] = "Standard";

// 默认的规约地方版本为 WFET1600UHW
char g_termtype_str[20] = "WFET1600UHW";

ConfigFile::ConfigFile()
{
    PrgList = 0;
    CurrNode = 0;
}

ConfigFile::~ConfigFile()
{
    Clear();
}


void ConfigFile::Read( char const * FileName ) 
{
    FILE * fp;
    char line[160];
    char delimiter[] = " =,\t\n";
    char *token;
    int count = 0;
    int i;

    if ( FileName == 0 || strlen( FileName ) == 0 )
    {
        if ( access( DefaultConfName, R_OK ) == 0 )
        {
            FileName = DefaultConfName;
        }
    }

    fp = fopen( FileName, "r" );

    if ( fp == 0 ) 
    {
        fprintf( stderr, "can not open watchdog config file: %s", FileName );
        return;
    }

    //to set default reboot interval -- 10days
    rebootinterval = 10 * 3600 * 24;
  
    fseek( fp, 0L, SEEK_SET );

    while( !feof( fp ) ) 
    {
        struct ProgramList *curr;

        fgets( line, sizeof( line ), fp );
        char *last;
        token = strtok_r( line, delimiter, &last );

        if ( token == 0 || token[0] == '#' ) 
        {
            continue;
        }

        if ( ( strcmp( token, "reboot" ) != 0 ) && 
            ( strcmp( token, "rerun" ) != 0 ) && 
            ( strcmp( token, "once" ) != 0 ) &&
            ( strcmp( token, "TTYPE" ) != 0 ) &&
            ( strcmp( token, "locale" ) != 0 ) &&
            ( strcmp( token, "cputype" ) != 0 )) 
        {
            continue;
        }

        if ( strcmp( token, "TTYPE" ) == 0 ) 
        {
            token = strtok_r( 0, delimiter, &last );
            if ( strlen ( token ) > 0 ) 
            {
                strncpy( g_termtype_str, token, 20 );
                g_termtype_str[19] = 0;
            }
            continue;
        }

        if ( strcmp( token, "locale" ) == 0 ) 
        {
            token = strtok_r( 0, delimiter, &last );
            if ( strlen ( token ) > 0 ) 
            {
                strncpy( g_locale_str, token, 20 );
                g_locale_str[19] = 0;
            }
            continue;
        }
        
        if ( strcmp( token, "cputype" ) == 0 ) 
        {
            token = strtok_r( 0, delimiter, &last );
            if ( strlen ( token ) > 0 ) 
            {
                strncpy( g_cputype_str, token, 20 );
                g_cputype_str[19] = 0;
            }
            continue;
        }   
 
        curr = ( struct ProgramList * )malloc( sizeof( struct ProgramList ) );

        /* to set program run parameters */
        if ( strcmp( token, "reboot" ) == 0 ) 
        {
            curr->node.mode = REBOOT;
        }
        else if ( strcmp( token, "rerun" ) == 0 )
        {
            curr->node.mode = RERUN;
        }
        else 
        {
            curr->node.mode = ONCE;
        }

        token = strtok_r( 0, delimiter, &last );
        if ( token == 0 ) 
        {
            free( curr );
            continue;
        }
        curr->node.path = strdup( token );

        for ( i = 0; i < (int)(sizeof(curr->node.param)/sizeof(char *)); i ++ ) 
        {
            token = strtok_r( 0, delimiter, &last );
            if ( token == 0 ) 
            {
                curr->node.param[i] = 0;
                break;
            }
            curr->node.param[i] = strdup( token );
        }

        count ++;
        curr->next = PrgList;
        PrgList = curr;
    }

    fclose( fp );
    return;
}

struct Program * ConfigFile::FirstProg( )
{  
    CurrNode = PrgList;

    return CurrProg();
}

struct Program * ConfigFile::NextProg( )
{
    if ( CurrNode == 0 ) 
    {
        return 0;
    }

    CurrNode = CurrNode->next;

    if ( CurrNode == 0 ) 
    {
        return 0;
    }

    return CurrProg();
}

struct Program * ConfigFile::CurrProg()
{
    if ( CurrNode == 0 ) 
    {
        return 0;
    }

    return &( CurrNode->node );
}

void ConfigFile::Clear()
{
    int i;
    struct ProgramList *tmp = PrgList;

    while( PrgList != 0 ) 
    {
        for ( i = 0; i < 10; i ++ ) 
        {
            if ( PrgList->node.param[i] == 0 ) 
            {
                break;
            }
            free( PrgList->node.param[i] );
        }
        free( PrgList->node.path );
        tmp = PrgList ;
        PrgList = PrgList->next;
        free( tmp );
    }

    CurrNode = 0;
    PrgList = 0;
}

