#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <stdio.h>
#include <string.h>

#include "structs.h"

class SettingsManager
{
public:
    time_t t_ignore;
    time_t t_check;

    SettingsManager();
    ~SettingsManager();

    int         SaveSettings();
    int         LoadSettings();
    int         GetTimerInterval();
    int         GetFirstCheckInterval();
    int         GetNotificationTimeout();
    char*       GetSoundFile();
    int         GetDebugLevel();
    const char *GetLastCommand();
    void        SetTimerInterval(int interval);
    void        SetFirstCheckInterval(int interval);
    void        SetNotificationTimeout(int interval);
    void        SetSoundFile(char *sfile);
    void        SetDebugLevel(int level);
    void        SetLastCommand(const char *);
    int         GetLocation();
    void        SetLocation(int index);
    int         GetDecreaseFont();
    void        SetDecreasefont(int val);

protected:
    settings*   psettings;
    char*       buffer2value_s(char *buffer, char *SearchPattern, char *section);
    char*       GetSettingsFilePath();
    char*       s_lastcommand;
};

#endif // SETTINGSMANAGER_H
