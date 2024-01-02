#include <wax/libwax.h>


/*
 * because 'rename()' cannot cross the filesystem, so use 'cp' and 'rm' instead
 */
int mv(const char *old, const char *new) {
    int ret;

    ret = cp_r(old, new);
    if (ret < 0)
        return ret;

    ret = rm_r(old);
    if (ret < 0)
        return ret;

    return 0;
}
