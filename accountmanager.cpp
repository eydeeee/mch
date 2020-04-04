#define _CRT_SECURE_NO_WARNINGS

#include "accountmanager.h"
#include <QDir>

AccountManager::AccountManager()
{
    cnt = 0;
    plist = 0;

    LoadAccountData();
}

AccountManager::~AccountManager()
{
}

int AccountManager::SaveAccountData()
{
    char *ptr;
    QString path = ptr = GetSettingsFilePath();
    if(ptr) free(ptr);
    QDir *dir = new QDir(path);
    dir->mkdir(path);
    delete dir;
#ifdef _WIN32
    SetFileAttributes(path.toStdWString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif

    path += "/accounts.ini";

    if(cnt<1)
    {
        remove(path.toStdString().c_str());
        return 0;
    }

    FILE *fp = fopen(path.toStdString().c_str(), "wt");
    if(!fp) return -1;

    fprintf(fp, "count = %d\n", cnt);

    for(int i=0; i<cnt; i++)
    {
        char psw[256];
        strcpy(psw, plist[i].s_password);
        scramble(psw);

        fprintf(fp, "[Account%d]\nFriendlyName = \"%s\"\n", i, plist[i].s_friendlyname);
        fprintf(fp, "LoginName = \"%s\"\n", plist[i].s_loginname);
        fprintf(fp, "Password = \"%s\"\n", psw);
        fprintf(fp, "Domain = \"%s\"\n", plist[i].s_domain);
        fprintf(fp, "Command = \"%s\"\n", plist[i].s_command);
        fprintf(fp, "Port = \"%d\"\n", plist[i].i_port);
        fprintf(fp, "Secure = \"%d\"\n\n", plist[i].i_ssl);
    }

    fclose(fp);

    LoadAccountData();

    return 1;
}

int AccountManager::LoadAccountData()
{
    char *ptr = GetSettingsFilePath();
    QString path = QString(ptr) + QString("/accounts.ini");
    if(ptr) free(ptr);

    int read;
    char firstline[256];

    FILE *fp = fopen(path.toStdString().c_str(), "rt");
    if(!fp) return -1;

    char *s = fgets(firstline, 256, fp);
    if(s){}
    char *p = strstr(firstline, "=");
    if(!p) return -1;
    while(p[0] == '=' || p[0] == ' ' || p[0] == '\"') p++;
    cnt = atoi(p);

    if(plist) plist = (acclist*)realloc(plist, cnt * sizeof(acclist));
    else plist = (acclist*)malloc(cnt * sizeof(acclist));
    if(!plist)
    {
        fclose(fp);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);

    int size = 0;
    while(1)
    {
        int read = fgetc(fp);
        if(read != EOF) size++; else break;
    }

    char *buffer = (char*)calloc(size, sizeof(char));
    fseek(fp, 0, SEEK_SET);

    read = fread(buffer, 1, size, fp);
    if(read != size)
    {
        if(buffer) free(buffer);
        fclose(fp);
        return -1;
    }

    for(int i=0; i<cnt; i++)
    {
        char *p;

        p = buffer2value_s(buffer, (char*)"Friend", i);
        if(p)
        {
            strcpy(plist[i].s_friendlyname, p);
            free(p);
        }
        else strcpy(plist[i].s_friendlyname, (char*)"***missing***");

        p = buffer2value_s(buffer, (char*)"Login", i);
        if(p)
        {
            strcpy(plist[i].s_loginname, p);
            free(p);
        }
        else strcpy(plist[i].s_loginname, (char*)"***missing***");

        p = buffer2value_s(buffer, (char*)"Password", i);
        if(p)
        {
            strcpy(plist[i].s_password, unscramble(p));
            free(p);
        }
        else strcpy(plist[i].s_password, (char*)"***missing***");

        p = buffer2value_s(buffer, (char*)"Domain", i);
        if(p)
        {
            strcpy(plist[i].s_domain, p);
            free(p);
        }
        else strcpy(plist[i].s_domain, (char*)"***missing***");

        p = buffer2value_s(buffer, (char*)"Command", i);
        if(p)
        {
            strcpy(plist[i].s_command, p);
            free(p);
        }
        else strcpy(plist[i].s_command, (char*)"***missing***");


        p = buffer2value_s(buffer, (char*)"Port", i);
        if(p)
        {
            plist[i].i_port = atoi(p);
            free(p);
        }
        else plist[i].i_port = 993;

        p = buffer2value_s(buffer, (char*)"Secure", i);
        if(p)
        {
            plist[i].i_ssl = atoi(p);
            free(p);
        }
        else plist[i].i_ssl = 1;
    }

    if(buffer) free(buffer);
    fclose(fp);

    return 1;
}

int AccountManager::AddAccount(acclist *paccount)
{
    if(plist) plist = (acclist*)realloc(plist, (++cnt) * sizeof(acclist));
    else
    {
        plist = (acclist*)malloc(sizeof(acclist));
        cnt = 1;
    }

    if(!plist) return -1;

    plist[cnt-1] = (*paccount);

    return 1;
}

int AccountManager::DeleteAccount(int index)
{
    int offset =0;
    acclist *pnew = (acclist*)calloc(--cnt, sizeof(acclist));
    if(!pnew) return -1;

    for(int i=0; i<cnt; i++)
    {
        if(i==index) offset = 1;
        pnew[i] = plist[i+offset];
    }

    free(plist); plist = pnew;

    return 1;
}

int AccountManager::GetCount()
{
    return cnt;
}

void AccountManager::SetFriendlyName(int index, const char *name)
{
    strcpy(plist[index].s_friendlyname, name);
}

void AccountManager::SetLoginName(int index, const char *name)
{
    strcpy(plist[index].s_loginname, name);
}

void AccountManager::SetPassword(int index, const char *psw)
{
    strcpy(plist[index].s_password, psw);
}

void AccountManager::SetDomain(int index, const char *domain)
{
    strcpy(plist[index].s_domain, domain);
}

void AccountManager::SetCommand(int index, const char *command)
{
    strcpy(plist[index].s_command, command);
}

void AccountManager::SetPort(int index, int port)
{
    plist[index].i_port = port;
}

void AccountManager::SetSSL(int index, int ssl)
{
    plist[index].i_ssl = ssl;
}

char* AccountManager::GetFriendlyName(int index)
{
    if(index<0 || index>cnt-1) return (char*)"Invalid index!";
    char *ret = (char *)calloc(256, sizeof(char));
    strcpy(ret, plist[index].s_friendlyname);
    return ret;
}

char* AccountManager::GetLoginName(int index)
{
    if(index<0 || index>cnt-1) return (char*)"Invalid index!";
    char *ret = (char *)calloc(256, sizeof(char));
    strcpy(ret, plist[index].s_loginname);
    return ret;
}

char* AccountManager::GetPassword(int index)
{
    if(index<0 || index>cnt-1) return (char*)"Invalid index!";
    char *ret = (char *)calloc(256, sizeof(char));
    strcpy(ret, plist[index].s_password);
    return ret;
}

char* AccountManager::GetDomain(int index)
{
    if(index<0 || index>cnt-1) return (char*)"Invalid index!";
    char *ret = (char *)calloc(256, sizeof(char));
    strcpy(ret, plist[index].s_domain);
    return ret;
}

char* AccountManager::GetCommand(int index)
{
    if(index<0 || index>cnt-1) return (char*)"Invalid index!";
    char *ret = (char *)calloc(256, sizeof(char));
    strcpy(ret, plist[index].s_command);
    return ret;
}

int AccountManager::GetPort(int index)
{
    if(index<0 || index>cnt-1) return -1;
    return plist[index].i_port;
}

int AccountManager::GetSSL(int index)
{
    if(index<0 || index>cnt-1) return -1;
    return plist[index].i_ssl;
}

acclist * AccountManager::GetAccountPtr(int index)
{
    return &plist[index];
}

char * AccountManager::scramble(char *text)
{
    for(int i=0;i<(int)strlen(text);i++)
    {
        if(i%2) text[i] = text[i]-3;
        else text[i] = text[i]+2;
    }
    return text;
}

char * AccountManager::unscramble(char *text)
{
    for(int i=0;i<(int)strlen(text);i++)
    {
        if(i%2) text[i] = text[i]+3;
        else text[i] = text[i]-2;
    }
    return text;
}

char * AccountManager::buffer2value_s(char *buffer, char *SearchPattern, int accnum)
{
    char *str, *base, *p1, *p2, *lend;

    str = (char*)calloc(256, sizeof(char));

    sprintf(str, "[Account%d]", accnum);
    base = strstr(buffer, str);
    p1 = strstr(base, SearchPattern);
    if(!p1) return 0;
    lend = strstr(base+1, "[");
    if(!lend) lend = strchr(p1, '\0');
    if(!lend) return 0;
    if(p1>lend) return 0;

    p1 = strstr(p1, "="); p1++; while(p1[0]==' ' || p1[0] == '\"') p1++;
    p2 = strstr(p1, "\n"); while(p2[-1] == ' ' || p2[-1] == '\"') p2--;
    strncpy(str, p1, p2-p1); str[p2-p1] = '\0';

    return str;
}

char* AccountManager::GetSettingsFilePath()
{
    QString s = QDir::homePath() + "/.mchecker";
    char *ret = (char *)calloc(256, sizeof(char));
    strcpy(ret, s.toStdString().c_str());

    return ret;
}
