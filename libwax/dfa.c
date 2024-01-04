#include <stddef.h>
#include <stdlib.h>
#include <string.h>


struct dfa
{
    int i;
    char *str;
    int regex_len;
};


struct dfa *new_dfa(char *regex)
{
    struct dfa *d = calloc(1, sizeof(struct dfa));
    d->str = strdup(regex);
    d->regex_len = strlen(regex);
    return d;
}


// 'buf' is a char array, may not endup with '\0'
int dfa_check(struct dfa *d, char *buf, size_t len)
{
    char ch;

    for (int i = 0; i < len; ++i) {
        if (d->i == d->regex_len)
            break;

        ch = buf[i];
        if (d->str[d->i] == ch)
            ++(d->i);
        else
            d->i = 0;
    }

    if (d->i == d->regex_len)
        return 0;

    return 1;
}


void dfa_reset(struct dfa *d)
{
    d->i = 0;
}


void dfa_delete(struct dfa *d)
{
    free(d->str);
    free(d);
}
