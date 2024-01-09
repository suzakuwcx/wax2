#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

extern int errno;


/*
 * check if each char of a string is a digital, true if all char is digital
 */
bool is_string_number(const char *str)
{
    size_t len = strlen(str);

    for (int i = 0; i < len; ++i) {
        if (isdigit(str[i]) == 0)
            return false;
    }

    return true;
}


bool is_string_bool(const char *str)
{
    return strcmp("true", str) == 0 || strcmp("false", str) == 0;
}


bool string_to_bool(const char *str)
{
    if (strcmp("true", str) == 0)
        return true;
    else
        return false;
}


int mkdir_p(const char *path)
{
    int ret = 0;
    int len = strlen(path);
    char *p = calloc(len + 2, sizeof(char));

    /* add a trailing slash */
    ++len;
    snprintf(p, len + 1, "%s/", path);

    for (int i = 0; i < len; ++i) {
        if (p[i] != '/')
            continue;

        p[i] = '\0';
        
        if (access(p, F_OK) != 0 && strlen(p) != 0) {
            ret = mkdir(p, 0777);
            if (ret < 0){
                fprintf(stderr, "cannot create directory \'%s\': %s\n", p, strerror(errno));
                goto clean;
            }
        }

        p[i] = '/';
    }

clean:
    free(p);
    return ret;
}


int systemf(const char *restrict format, ...)
{
    va_list args;
    char command[512];

    memset(command, 0, sizeof(command));

    va_start(args, format);
    vsprintf(command, format, args);
    va_end(args);

    return system(command);
}

void clear_screen()
{
    printf("\33[H\33[2J\r");
}

void clear_line()
{
    printf("\33[2K\r");
}