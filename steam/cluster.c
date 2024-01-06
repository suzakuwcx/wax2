#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <wax/conf.h>
#include <wax/libwax.h>

extern int errno;

int cluster_list(struct vector *vec)
{
    DIR *dirp = NULL;
    struct dirent *entry;

    dirp = opendir(config_get_dst_cluster_dir());
    if (dirp == NULL) {
        fprintf(stderr, "cannot open cluster directory %s: %s\n", config_get_dst_cluster_dir(), strerror(errno));
        return -EPERM;
    }

    while ((entry = readdir(dirp)) != NULL) {
        if ((strcmp(".", entry->d_name) == 0) || (strcmp("..", entry->d_name) == 0))
            continue;

        vector_push(vec, entry->d_name, sizeof(((struct dirent *)NULL)->d_name));
    }

    return 0;
}

int cluster_create(const char *name)
{
    char path[PATH_MAX];

    memset(path, 0, sizeof(path));
    snprintf(path, sizeof(path), "%s/%s", config_get_dst_cluster_dir(), name);

    return mkdir_p(path);
}