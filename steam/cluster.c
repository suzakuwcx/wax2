#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <wax/conf.h>
#include <wax/libwax.h>
#include <wax/vector.h>

#include <steam/cluster.h>

extern int errno;


static const char cluster_ini_fmt[] = 
"[GAMEPLAY]\n"
"game_mode = %s\n"
"max_players = %d\n"
"pvp = %s\n"
"vote_enabled = %s\n"
"\n"
"[NETWORK]\n"
"cluster_name = %s\n"
"cluster_description = %s\n"
"cluster_password = %s\n"
"cluster_intention = cooperative\n"
"lan_only_cluster = false\n"
"offline_cluster = false\n"
"autosaver_enabled = true\n"
"enable_vote_kick = false\n"
"tick_rate = 15\n"
"connection_timeout = 8000\n"
"\n"
"[MISC]\n"
"max_snapshots = 6\n"
"console_enabled = true\n"
"\n"
"[SHARD]\n"
"shard_enabled = true\n"
"bind_ip = 127.0.0.1\n"
"master_ip = 127.0.0.1\n"
"master_port = 10888\n"
"cluster_key = defaultPass\n",

master_server_ini[] = 
"[NETWORK]\n"
"server_port = 10999\n"
"\n"
"[SHARD]\n"
"is_master = true\n"
"\n"
"[ACCOUNT]\n"
"encode_user_path = true\n",

caves_server_ini[] = 
"[NETWORK]\n"
"server_port = 10998\n"
"\n"
"[SHARD]\n"
"is_master = false\n"
"name = Caves\n"
"\n"
"[ACCOUNT]\n"
"encode_user_path = true\n";


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
    FILE *fp;
    char path[PATH_MAX];
    char tmp[PATH_MAX];

    memset(path, 0, sizeof(path));
    memset(tmp, 0, sizeof(path));
    snprintf(path, sizeof(path), "%s/%s", config_get_dst_cluster_dir(), name);

    if (access(path, F_OK) == 0)
        return 0;

    snprintf(tmp, sizeof(tmp), "%s/Master", path);
    mkdir_p(tmp);

    snprintf(tmp, sizeof(tmp), "%s/Caves", path);
    mkdir_p(tmp);

    snprintf(tmp, sizeof(tmp), "%s/cluster.ini", path);
    fp = fopen(tmp, "w+");
    fprintf(fp, cluster_ini_fmt, "survival", 6, "false", "true", "wax2", "wax2", "wax2");
    fclose(fp);

    snprintf(tmp, sizeof(tmp), "%s/Master/server.ini", path);
    fp = fopen(tmp, "w+");
    fprintf(fp, "%s", master_server_ini);
    fclose(fp);

    snprintf(tmp, sizeof(tmp), "%s/Caves/server.ini", path);
    fp = fopen(tmp, "w+");
    fprintf(fp, "%s", caves_server_ini);
    fclose(fp);

    return 0;
}
