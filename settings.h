#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct Settings {
    int width;
    int height;
    int mines;
} Settings;

Settings settingsLoad(const char *path, Settings defaults);

#endif
