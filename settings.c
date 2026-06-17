#include "settings.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *trim(char *text) {
    while (isspace((unsigned char) *text)) {
        ++text;
    }

    if (*text == '\0') {
        return text;
    }

    char *end = text + strlen(text) - 1;
    while (end > text && isspace((unsigned char) *end)) {
        *end = '\0';
        --end;
    }

    return text;
}

static int parseInt(const char *text, int *outValue) {
    char *end = NULL;
    const long value = strtol(text, &end, 10);

    if (text == end) {
        return 0;
    }

    while (end && isspace((unsigned char) *end)) {
        ++end;
    }

    if (end && *end != '\0') {
        return 0;
    }

    *outValue = (int) value;
    return 1;
}

Settings settingsLoad(const char *path, Settings defaults) {
    Settings settings = defaults;

    FILE *file = fopen(path, "r");
    if (!file) {
        return settings;
    }

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        char *text = trim(line);

        if (*text == '\0' || *text == '#' || *text == ';') {
            continue;
        }

        char *comment = strchr(text, '#');
        if (comment) {
            *comment = '\0';
        }

        comment = strchr(text, ';');
        if (comment) {
            *comment = '\0';
        }

        char *equals = strchr(text, '=');
        if (!equals) {
            continue;
        }

        *equals = '\0';

        char *key = trim(text);
        char *valueText = trim(equals + 1);

        int value = 0;
        if (!parseInt(valueText, &value)) {
            continue;
        }

        if (strcmp(key, "width") == 0) {
            settings.width = value;
        } else if (strcmp(key, "height") == 0) {
            settings.height = value;
        } else if (strcmp(key, "mines") == 0) {
            settings.mines = value;
        }
    }

    fclose(file);

    if (settings.width < 1) {
        settings.width = defaults.width;
    }

    if (settings.height < 1) {
        settings.height = defaults.height;
    }

    const int maxMines = settings.width * settings.height - 1;
    if (settings.mines < 1) {
        settings.mines = defaults.mines;
    } else if (settings.mines > maxMines) {
        settings.mines = maxMines;
    }

    return settings;
}
