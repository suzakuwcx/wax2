#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

extern int errno;

int rm_r(const char *path)
{
    int ret;
    struct stat st;
    struct dirent *entry;
    DIR *dirp;

    char buff[PATH_MAX];

    memset(&st, 0, sizeof(st));
    lstat(path, &st);

    if (access(path, F_OK) != 0)
        return -EPERM;

    memset(buff, 0, sizeof(buff));

    if (S_ISDIR(st.st_mode)) {
        dirp = opendir(path);
        while ((entry = readdir(dirp)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(buff, sizeof(buff), "%s/%s", path, entry->d_name);
            rm_r(buff);
        }
        rmdir(path);
        if (ret < 0) {
            fprintf(stderr, "cannot remove directory %s: %s", path, strerror(errno));
            return ret;
        }
    } else if (S_ISLNK(st.st_mode)) {
        ret = unlink(path);
        if (ret < 0) {
            fprintf(stderr, "cannot unlink symbol link %s: %s\n", path, strerror(errno));
            return ret;
        }
    } else if (S_ISREG(st.st_mode)) {
        ret = remove(path);
        if (ret < 0) {
            fprintf(stderr, "cannot remove file %s: %s\n", path, strerror(errno));
            return ret;
        }
    }
    else {
        fprintf(stderr, "file type not support \n");
        return -EPERM;
    }

    return 0;
}
