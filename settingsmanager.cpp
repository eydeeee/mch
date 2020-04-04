#define _CRT_SECURE_NO_WARNINGS

#include "settingsmanager.h"
#include <QDir>

SettingsManager::SettingsManager()
{
    psettings = (settings*)calloc(1, sizeof(settings));
    psettings->i_interval = 120;
    psettings->i_firstcheckinterval = 10;
    psettings->i_notificationtimeout = 10;
    strcpy(psettings->s_sound, "./snd/notify.wav");
    s_lastcommand = 0;
    t_check = 0;
    t_ignore = 0;
}

SettingsManager::~SettingsManager()
{
    if(psettings) free(psettings);
    if(s_lastcommand) free(s_lastcommand);
}

int SettingsManager::SaveSettings()
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

    path += "/settings.ini";
    FILE *fp = fopen(path.toStdString().c_str(), "wt");
    if(!fp) return -1;

    fprintf(fp, "[Main]\n");
    fprintf(fp, "TimerInterval = \"%d\"\n", psettings->i_interval);
    fprintf(fp, "FirstCheckInterval = \"%d\"\n", psettings->i_firstcheckinterval);
    fprintf(fp, "NotificationTimeout = \"%d\"\n", psettings->i_notificationtimeout);
    fprintf(fp, "SoundFile = \"%s\"\n", psettings->s_sound);

    fprintf(fp, "\n[Misc]\n");
    fprintf(fp, "WindowLocation = \"%d\"\n", psettings->i_location);
    fprintf(fp, "DecreaseFontSize = \"%d\"\n", psettings->i_decreasefont);
    fprintf(fp, "DebugLevel = \"%d\"\n", psettings->i_debug);

    fclose(fp);

    LoadSettings();

    return 1;
}

int SettingsManager::LoadSettings()
{
    char *ptr = GetSettingsFilePath();
    QString path = QString(ptr) + QString("/settings.ini");
    if(ptr) free(ptr);
    FILE *fp = fopen(path.toStdString().c_str(), "rt");
    if(!fp) return -1;

    int size = 0, read = 0;
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

    char *p;

    p = buffer2value_s(buffer, (char*)"Sound", (char*)"Main");
    if(p)
    {
        strcpy(psettings->s_sound, p);
        free(p);
    }
    else strcpy(psettings->s_sound, "./snd/notify.wav");

    p = buffer2value_s(buffer, (char*)"Timer", (char*)"Main");
    if(p)
    {
        psettings->i_interval = atoi(p);
        free(p);
    }
    else psettings->i_interval = 120;

    p = buffer2value_s(buffer, (char*)"FirstCheck", (char*)"Main");
    if(p)
    {
        psettings->i_firstcheckinterval = atoi(p);
        free(p);
    }
    else psettings->i_firstcheckinterval = 10;

    p = buffer2value_s(buffer, (char*)"NotificationTime", (char*)"Main");
    if(p)
    {
        psettings->i_notificationtimeout = atoi(p);
        free(p);
    }
    else psettings->i_notificationtimeout = 10;

    p = buffer2value_s(buffer, (char*)"WindowLocation", (char*)"Misc");
    if(p)
    {
        psettings->i_location = atoi(p);
        free(p);
    }
    else psettings->i_location = 0;

    p = buffer2value_s(buffer, (char*)"DecreaseFontSize", (char*)"Misc");
    if(p)
    {
        psettings->i_decreasefont = atoi(p);
        free(p);
    }
    else psettings->i_decreasefont = 0;

    p = buffer2value_s(buffer, (char*)"Debug", (char*)"Misc");
    if(p)
    {
        psettings->i_debug = atoi(p);
        free(p);
    }
    else psettings->i_debug = 0;

    if(buffer) free(buffer);
    fclose(fp);

    return 1;
}

int SettingsManager::GetTimerInterval()
{
    return psettings->i_interval;
}

int SettingsManager::GetFirstCheckInterval()
{
    return psettings->i_firstcheckinterval;
}

int SettingsManager::GetNotificationTimeout()
{
    return psettings->i_notificationtimeout;
}

char* SettingsManager::GetSoundFile()
{
    return psettings->s_sound;
}

int SettingsManager::GetDebugLevel()
{
    return psettings->i_debug;
}

void SettingsManager::SetTimerInterval(int interval)
{
    psettings->i_interval = interval;
}

void SettingsManager::SetFirstCheckInterval(int interval)
{
    psettings->i_firstcheckinterval = interval;
}

void SettingsManager::SetNotificationTimeout(int interval)
{
    psettings->i_notificationtimeout = interval;
}

void SettingsManager::SetSoundFile(char *sfile)
{
    strcpy(psettings->s_sound, sfile);
}

void SettingsManager::SetDebugLevel(int level)
{
    psettings->i_debug = level;
}

int SettingsManager::GetLocation()
{
    return psettings->i_location;
}

void SettingsManager::SetLocation(int index)
{
    psettings->i_location = index;
}

int SettingsManager::GetDecreaseFont()
{
    return psettings->i_decreasefont;
}

void SettingsManager::SetDecreasefont(int val)
{
    psettings->i_decreasefont = val;
}

char* SettingsManager::buffer2value_s(char *buffer, char *SearchPattern, char *section)
{
    char *str, *base, *p1, *p2, *lend;

    str = (char*)calloc(256, sizeof(char));

    sprintf(str, "[%s]", section);
    base = strstr(buffer, str);
    p1 = strstr(base, SearchPattern);
    if(!p1) return 0;
    lend = strstr(base+1, "[");
    if(!lend) lend = strchr(p1, '\0');
    if(!lend) return 0;
    if(p1>lend) return 0;

    p1 = strstr(p1, "="); p1++; while(p1[0]==' ' || p1[0] == '\"') p1++;
    p2 = strstr(p1, "\n"); while(p2[-1] == ' ' || p2[-1] == '\"') p2--;
    if(p1>p2) return 0;
    strncpy(str, p1, p2-p1); str[p2-p1] = '\0';

    return str;
}

char* SettingsManager::GetSettingsFilePath()
{
    QString s = QDir::homePath() + "/.mchecker";
    char *ret = (char *)calloc(256, sizeof(char));
    strcpy(ret, s.toStdString().c_str());

    return ret;
}

const char* SettingsManager::GetLastCommand()
{
    return (const char*)s_lastcommand;
}

void SettingsManager::SetLastCommand(const char *cmd)
{
    if(!s_lastcommand) s_lastcommand = (char*)calloc(strlen(cmd)+1, sizeof(char));
    else s_lastcommand = (char*)realloc(s_lastcommand, (strlen(cmd)+1) * sizeof(char));
    strcpy(s_lastcommand, cmd);
}
