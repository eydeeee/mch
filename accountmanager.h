#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <stdio.h>
#include <string.h>

#include "structs.h"

class AccountManager
{
public:
    AccountManager();
    ~AccountManager();

    int     SaveAccountData();
    int     LoadAccountData();
    int     AddAccount(acclist *paccount);
    int     DeleteAccount(int index);
    int     GetCount();
    void    SetFriendlyName(int index, const char *name);
    void    SetLoginName(int index, const char *name);
    void    SetPassword(int index, const char *psw);
    void    SetDomain(int index, const char *domain);
    void    SetCommand(int index, const char *command);
    void    SetPort(int index, int port);
    void    SetSSL(int index, int ssl);
    char*   GetFriendlyName(int index);
    char*   GetLoginName(int index);
    char*   GetPassword(int index);
    char*   GetDomain(int index);
    char*   GetCommand(int index);
    int     GetPort(int index);
    int     GetSSL(int index);
    acclist *GetAccountPtr(int index);

protected:
    acclist *plist;
    int     cnt;
    char *  scramble(char *text);
    char *  unscramble(char *text);
    char *  buffer2value_s(char *buffer, char *SearchPattern, int accnum);
    char*   GetSettingsFilePath();
};

#endif // ACCOUNTMANAGER_H
