#include <dirent.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <wax/conf.h>
#include <wax/libwax.h>
#include <wax/vector.h>
#include <wax/conf-file.h>

#include <steam/cluster.h>

extern int errno;

static struct cluster_conf *conf;
static struct vector *gamemode_chosen_list;

struct cluster_conf {
    char *path;
    /* [GAMEPLAY] */
    enum game_mode mode;
    unsigned int max_player;
    bool pvp;
    bool vote_enable;
    
    /* [NETWORK] */
    char *server_name;
    char *server_description;
    char *server_password;
};


static void __attribute__((constructor)) init_module_cluster()
{
    gamemode_chosen_list = new_vector(NULL);
    vector_push(gamemode_chosen_list, "survival", 0);
    vector_push(gamemode_chosen_list, "endless", 0);
    vector_push(gamemode_chosen_list, "wilderness", 0);
}


static void __attribute__((destructor)) module_conf_exit()
{
    vector_delete(gamemode_chosen_list);
}


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

    struct cluster_conf *conf;

    memset(path, 0, sizeof(path));
    memset(tmp, 0, sizeof(path));
    snprintf(path, sizeof(path), "%s/%s", config_get_dst_cluster_dir(), name);

    if (access(path, F_OK) == 0)
        return 0;

    snprintf(tmp, sizeof(tmp), "%s/Master", path);
    mkdir_p(tmp);

    snprintf(tmp, sizeof(tmp), "%s/Master/server.ini", path);
    fp = fopen(tmp, "w+");
    fprintf(fp, "%s", master_server_ini);
    fclose(fp);

    snprintf(tmp, sizeof(tmp), "%s/Caves", path);
    mkdir_p(tmp);

    snprintf(tmp, sizeof(tmp), "%s/Caves/server.ini", path);
    fp = fopen(tmp, "w+");
    fprintf(fp, "%s", caves_server_ini);
    fclose(fp);

    /* create a default cluster.ini */
    conf = new_cluster_conf(name);
    cluster_conf_save(conf);
    cluster_conf_delete(conf);

    return 0;
}


struct cluster_conf *new_cluster_conf(const char *name)
{
    FILE *fp;
    struct cluster_conf *_conf;

    char path[PATH_MAX];

    memset(path, 0, sizeof(path));
    snprintf(path, sizeof(path), "%s/%s/%s", config_get_dst_cluster_dir(), name, "cluster.ini");

    fp = fopen(path, "r");
    if (fp == NULL)
        fp = fopen(path, "w+");

    if (fp == NULL) {
        fprintf(stderr, "cannot open cluster configuration file %s: %s\n", path, strerror(errno));
        return NULL;
    }

    _conf = calloc(1, sizeof(struct cluster_conf));
    _conf->path = strdup(path);
    _conf->server_name = strdup("wax");
    _conf->server_description = strdup("wax server");
    _conf->server_password = strdup("123456");

    set_parsing_cluster_ini_mode();

    /* send to lex and yacc */
    yyin = fp;
    conf = _conf;

    /* start parsing */
    parser_restart();
    fclose(fp);
    
    return conf;
}


int cluster_conf_save(struct cluster_conf *conf)
{
    FILE *fp;
    char path[PATH_MAX];

    memset(path, 0, sizeof(path));

    fp = fopen(conf->path, "w+");
    if (fp == NULL) {
        fprintf(stderr, "cannot open file %s: %s", conf->path, strerror(errno));
        return -1;
    }

    fprintf(fp, cluster_ini_fmt, cluster_get_gamemode(conf), cluster_get_max_players(conf), 
                                    conf->pvp ? "true" : "false", conf->vote_enable ? "true" : "false",
                                    conf->server_name, conf->server_description, conf->server_password);
    fclose(fp);

    return 0;
}


inline struct cluster_conf *cluster_get_current_conf()
{
    return conf;
}


void cluster_conf_delete(struct cluster_conf *conf)
{
    free(conf->path);
    free(conf->server_name);
    free(conf->server_description);
    free(conf->server_password);

    free(conf);
}


int cluster_set_mode(struct cluster_conf *conf, const char *value)
{
    if (strcmp("survival", value) == 0)
        conf->mode = GAMEMODE_SURVIVAL;
    else if (strcmp("endless", value) == 0)
        conf->mode = GAMEMODE_ENDLESS;
    else if (strcmp("wilderness", value) == 0)
        conf->mode = GAMEMODE_WILDERNESS;
    else
        return -1;
    return 0;
}


int cluster_set_max_player(struct cluster_conf *conf, const char *value)
{
    if (!is_string_number(value))
        return -1;

    conf->max_player = atoi(value);
    return 0;
}


int cluster_set_pvp(struct cluster_conf *conf, const char *value)
{
    if (!is_string_bool(value))
        return -1;
    conf->pvp = string_to_bool(value);
    return 0;
}


int cluster_set_vote(struct cluster_conf *conf, const char *value)
{
    if (!is_string_bool(value))
        return -1;
    conf->vote_enable = string_to_bool(value);
    return 0;
}


inline int cluster_set_server_name(struct cluster_conf *conf, const char *value)
{
    free(conf->server_name);
    conf->server_name = strdup(value);
    return 0;
}


inline int cluster_set_server_description(struct cluster_conf *conf, const char *value)
{
    free(conf->server_description);
    conf->server_description = strdup(value);
    return 0;
}


inline int cluster_set_server_password(struct cluster_conf *conf, const char *value)
{
    free(conf->server_password);
    conf->server_password = strdup(value);
    return 0;
}


int cluster_set_value(struct cluster_conf *conf, const char *key, const char *value)
{
    int ret = -1;

    if (strcmp("game_mode", key) == 0)
        ret = cluster_set_mode(conf, value);
    else if (strcmp("max_players", key) == 0)
        ret = cluster_set_max_player(conf, value);
    else if (strcmp("pvp", key) == 0)
        ret = cluster_set_pvp(conf, value);
    else if (strcmp("vote_enabled", key) == 0)
        ret = cluster_set_vote(conf, value);
    else if (strcmp("cluster_name", key) == 0)
        ret = cluster_set_server_name(conf, value);
    else if (strcmp("cluster_description", key) == 0)
        ret = cluster_set_server_description(conf, value);
    else if (strcmp("cluster_password", key) == 0)
        ret = cluster_set_server_password(conf, value);

    if (ret < 0)
        return -1;

    return 0;
}


inline const char *cluster_get_gamemode(struct cluster_conf *conf)
{
    switch (conf->mode) {
        case GAMEMODE_SURVIVAL:
            return "survival";
        case GAMEMODE_ENDLESS:
            return "endless";
        case GAMEMODE_WILDERNESS:
            return "wilderness";
        default:
            return "survival";
    }
}


inline struct vector *cluster_get_gamemode_chosen_list()
{
    return gamemode_chosen_list;
}


inline int cluster_get_max_players(struct cluster_conf *conf)
{
    return conf->max_player;
}

inline bool cluster_get_enable_pvp(struct cluster_conf *conf)
{
    return conf->pvp;
}

inline bool cluster_get_enable_rollback_vote(struct cluster_conf *conf)
{
    return conf->vote_enable;
}

inline const char *cluster_get_server_name(struct cluster_conf *conf)
{
    return conf->server_name;
}

inline const char *cluster_get_server_description(struct cluster_conf *conf)
{
    return conf->server_description;
}

const char *cluster_get_server_password(struct cluster_conf *conf)
{
    return conf->server_password;
}

