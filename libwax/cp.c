#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <dirent.h>

#include <wax/libwax.h>

extern int errno;

/*
 * dst must be file
 */
static int _cp_regular_file(const char *src, const char *dst)
{
    int ret;
    FILE *sp;
    FILE *dp;

    size_t len;
    char buff[256];

    sp = fopen(src, "r");
    if (sp == NULL) {
        fprintf(stderr, "cannot open %s: %s", src, strerror(errno));
        ret = -EPERM;
        goto clean;
    }

    dp = fopen(dst, "w+");
    if (dp == NULL) {
        fprintf(stderr, "cannot open %s: %s", dst, strerror(errno));
        ret = -EPERM;
        goto sp_clean;
    }

    memset(buff, 0, sizeof(buff));

    do {
        len = fread(buff, 1, sizeof(buff), sp);
        fwrite(buff, 1, len, dp);
    } while (len == sizeof(buff));

    fclose(dp);
sp_clean:
    fclose(sp);
clean:
    return 0;
}

/*
 * dst is symbol link location
 */
static int _cp_symlink(const char *src, const char *dst)
{
    size_t len;
    char link_path[PATH_MAX];

    memset(link_path, 0, sizeof(link_path));

    len = readlink(src, link_path, sizeof(link_path));
    if (len < 0) {
        perror("readlink");
        return -1;
    }

    symlink(link_path, dst);
    return 0;
}


/*
 * dst is file location, if file exist, it would be override
 */
static int _cp_file(const char *src, const char *dst)
{
    int ret;
    struct stat s_stat;

    memset(&s_stat, 0, sizeof(s_stat));

    lstat(src, &s_stat);

    if (S_ISREG(s_stat.st_mode)) {
        ret = _cp_regular_file(src, dst);
        if (ret < 0) {
            fprintf(stderr, "cannot copy regular file\n");
            return ret;
        }

        chmod(dst, s_stat.st_mode);
        chown(dst, s_stat.st_uid, s_stat.st_gid);
    } else if (S_ISLNK(s_stat.st_mode)){
        ret = _cp_symlink(src, dst);
        if (ret < 0) {
            fprintf(stderr, "cannot copy symbol link\n");
            return ret;
        }
    }
    else {
        fprintf(stderr, "file type not support, skip: %s\n", src);
    }

    return 0;
}

/*
 * copy src/ to dst/, and dst must not exist
 */
static int _cp_dir(const char *src, const char *dst)
{
    int ret;
    struct stat s_stat;
    DIR *dirp;
    struct dirent *entry;
    char sub_src[PATH_MAX];
    char sub_dst[PATH_MAX];

    memset(&s_stat, 0, sizeof(s_stat));
    stat(src, &s_stat);

    ret = mkdir(dst, s_stat.st_mode);
    if (ret < 0) {
        fprintf(stderr, "cannot create directory %s: %s\n", dst, strerror(errno));
        return -EPERM;
    }

    chown(dst, s_stat.st_uid, s_stat.st_gid);

    memset(sub_src, 0, sizeof(sub_src));
    memset(sub_dst, 0, sizeof(sub_dst));

    dirp = opendir(src);

    while ((entry = readdir(dirp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(sub_src, sizeof(sub_src), "%s/%s", src, entry->d_name);
        snprintf(sub_dst, sizeof(sub_dst), "%s/%s", dst, entry->d_name);

        lstat(sub_src, &s_stat);

        if(S_ISDIR(s_stat.st_mode))
            _cp_dir(sub_src, sub_dst);
        else
            _cp_file(sub_src, sub_dst);
    }

    return 0;
}


int cp_r(const char *src, const char *dst)
{
    int ret;
    struct stat s_stat, d_stat;

    char *d_dirc, *d_dname;
    char *s_basec, *s_bname;
    char buff[PATH_MAX];

    if (access(src, F_OK) != 0) {
        fprintf(stderr, "cannot access %s: %s\n", src, strerror(errno));
        return -1;
    }

    memset(&s_stat, 0, sizeof(s_stat));
    memset(&d_stat, 0, sizeof(d_stat));
    memset(buff, 0, sizeof(buff));

    lstat(src, &s_stat);

    s_basec = strdup(src);
    s_bname = basename(s_basec);

    d_dirc = strdup(dst);
    d_dname = dirname(d_dirc);
    

    if (access(dst, F_OK) != 0) {
        if (access(d_dname, F_OK) != 0) {
            fprintf(stderr, "cannot access %s: %s\n", d_dname, strerror(errno));
            ret = -EPERM;
            goto clean;
        }

        if (S_ISDIR(s_stat.st_mode))
            _cp_dir(src, dst);
        else
            _cp_file(src, dst);
    }
    else {
        lstat(dst, &d_stat);
        if (S_ISLNK(d_stat.st_mode)) {
            ret = readlink(dst, buff, sizeof(buff));
            if (ret < 0) {
                fprintf(stderr, "cannot read link %s: %s\n", dst, strerror(errno));
                goto clean;
            }

            char *tmp = strdup(buff);
            // relative path
            if (buff[0] != '/')
                snprintf(buff, sizeof(buff), "%s/%s/%s", d_dname, tmp, s_bname);
            else
                snprintf(buff, sizeof(buff), "%s/%s", tmp, s_bname);

            free(tmp);
        }
        else if (S_ISDIR(d_stat.st_mode)) {
            snprintf(buff, sizeof(buff), "%s/%s", dst, s_bname);            
        }
        else {
            fprintf(stderr, "file %s exist\n", dst);
            ret = -EPERM;
            goto clean;   
        }

        if (S_ISDIR(s_stat.st_mode))
            _cp_dir(src, buff);
        else
            _cp_file(src, buff);
    }

clean:
    free(s_basec);
    free(d_dirc);
    return ret;
}
