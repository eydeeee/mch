#ifndef STRUCTS_H
#define STRUCTS_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct datetime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} datetime;

typedef struct
{
    int socket;
    SSL *sslHandle;
    SSL_CTX *sslContext;
} connection;

typedef struct slist
{
    int msg;
    slist *plist;
} lista;

typedef struct snippet
{
    int msgid;
    char from[256];
    char subject[256];
    char size[256];
    datetime dt;
    int ignored;
} snippet;

typedef struct snippet_container
{
    int accid;
    char accname[256];
    char cmd[256];
    int count;
    snippet *snippets;
} snippet_container;

typedef struct acclist
{
    char s_friendlyname[256];
    char s_loginname[256];
    char s_password[256];
    char s_domain[256];
    char s_command[256];
    int i_port;
    int i_ssl;
} acclist;

typedef struct settings
{
    char s_sound[256];
    int i_interval;
    int i_debug;
    int i_firstcheckinterval;
    int i_notificationtimeout;
    int i_location;
    int i_decreasefont;
} settings;



#endif // STRUCTS_H
