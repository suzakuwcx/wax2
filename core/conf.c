/**
 * @cite https://gitlab.archlinux.org/pacman/pacman.git 18e49f2c97f0e33a645f364ed9de8e3da6c36d41 pacman.c:parseargs
 */

#include <curl/curl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>

#include <wax/conf.h>
#include <wax/libwax.h>
#include <wax/conf-file.h>

extern int errno;

struct config {
    /* directory variable */
    char *wax_dir;
    char *steam_dir;
    char *dst_app_dir;
    char *dst_cluster_dir;
    char *dst_server_binary_dir;
    char *dst_workshop_download_dir;

    /* file path variable */
    char *token_path;
    char *config_file_path;
    
    /* const variable */
    char *program_name;
    char *dst_server_exe_name;

    /* modifable variable */
    char *token;
    char *cluster_name;

    /* the ptr of first argument */
    char * const *argument_list;

    enum wax_op op;            /* Primary operation */
    enum wax_subop sub_op;     /* Secondary operation */
    
    bool help;
};

static struct config *conf;

static void __attribute__((constructor)) init_module_conf() 
{
    char buff[256];
    char *p;

    conf = calloc(1, sizeof(struct config));
    if (conf == NULL) {
        perror("cannot assign memory for config");
        exit(1);
    }

    conf->cluster_name = strdup("Cluster_1");

    p = getenv("HOME");
    if (p == NULL) {
        fprintf(stderr, "cannot get environment variable HOME\n");
        exit(1);
    }

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/.local/share/wax", p);
    conf->wax_dir = strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/Steam", p);
    conf->steam_dir = strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/.klei/DoNotStarveTogether", p);
    conf->dst_cluster_dir = strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/steamapps/common/Don't Starve Together Dedicated Server", conf->steam_dir);
    conf->dst_app_dir= strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/bin64", conf->dst_app_dir);
    conf->dst_server_binary_dir = strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/steamapps/workshop/content/322330", conf->steam_dir);
    conf->dst_workshop_download_dir = strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/cluster_token.txt", conf->wax_dir);
    conf->token_path = strdup(buff); 

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/wax.conf", conf->wax_dir);
    conf->config_file_path = strdup(buff);

    conf->dst_server_exe_name = strdup("dontstarve_dedicated_server_nullrenderer_x64");

    conf->token = calloc(1, TOKEN_MAXSIZE);

    mkdir_p(config_get_wax_dir());
    config_load();

    return;
}


static void __attribute__((destructor)) module_conf_exit()
{
    free(conf->wax_dir);
    free(conf->steam_dir);
    free(conf->dst_app_dir);
    free(conf->dst_cluster_dir);
    free(conf->dst_server_binary_dir);
    free(conf->dst_workshop_download_dir);

    free(conf->token_path);
    free(conf->config_file_path);
    
    free(conf->program_name);
    free(conf->dst_server_exe_name);

    free(conf->token);
    free(conf->cluster_name);

    free(conf);
} 


static void usage(const char *name)
{
	printf("Usage:  %s <operation> [...]\n", name);
	printf("Operations: \n");
    printf("    {-S --server} <options>\n");
	printf("    {-M --mod} <options>\n");
	printf("\n");
	printf("Use '%s {-h --help}' with an operationfor available options\n", name);
	printf("Examples:\n");
	printf("  -Mh                    show help message for mod\n");
    printf("\n");
	printf("Config file location is '%s' and '%s/Steam'\n", config_get_wax_dir(), getenv("HOME"));
}


static void usage_mod(const char *name)
{
	printf("Usage:  %s {-M --mod} [options] [id(s)]\n", name);
	printf("options: \n");
	printf("  -l, --list             list installed mod\n");
    printf("  -u, --upgrade          upgrade all mod\n");
    printf("  -m, --menu             open a tui menu to configure mod\n");
    printf("\n");
	printf("Examples:\n");
	printf("  -M 1242093526          install Mod via workshop id\n");
	printf("\n");
}


static void usage_server(const char *name)
{
	printf("Usage:  %s {-S --Server} [options]\n", name);
	printf("options: \n");
	printf("  -s, --start               start server\n");
    printf("  -c, --close               slose server\n");
    printf("  -u, --upgrade             upgrade server\n");
    printf("  -m, --menu                open a tui menu to configure server\n");
    printf("\n");
    printf("      --cluster <cluster>   set cluster name\n");
	printf("\n");
}


static int parseargs_op(int opt)
{
    switch (opt) {
        case 'S':
            conf->op = CONFIG_OP_SERVER;
            break;
        case 'M':
            conf->op = CONFIG_OP_MOD;
            break;
        case 'h':
            conf->help = true;
            break;
        default:
            return 1;
    }
    
    return 0;
}


static int parseargs_mod(int opt)
{
    switch (opt) {
        case 'l':
            conf->sub_op = CONFIG_SUBOP_LIST;
            break;
        case 'u':
            conf->sub_op = CONFIG_SUBOP_UPGRADE;
            break;
        case 'm':
            conf->sub_op = CONFIG_SUBOP_MENU;
            break;
        default:
            return -EPERM;
    }
    
    return 0;
}


static int parseargs_server(int opt)
{
    switch (opt) {
        case 's':
            conf->sub_op = CONFIG_SUBOP_START;
            break;
        case 'c':
            conf->sub_op = CONFIG_SUBOP_STOP;
            break;
        case 'u':
            conf->sub_op = CONFIG_SUBOP_UPGRADE;
            break;
        case 'm':
            conf->sub_op = CONFIG_SUBOP_MENU;
            break;
        case OP_CLUSTER:
            conf->sub_op = CONFIG_SUBOP_NONE;
            free(conf->cluster_name);
            conf->cluster_name = strdup(optarg);
            break;
        default:
            return -EPERM;
    }
    
    return 0;
}


int parseargs(int argc, char *argv[])
{
	int c;
	int option_index = 0;

	const char *optstring = "SMhlumsc";

	static const struct option opts[] =
	{
		{"server", no_argument, 0, 'S'},
		{"mod", no_argument, 0, 'M'},
		{"help", no_argument, 0, 'h'},

        {"list", no_argument, 0, CONFIG_SUBOP_LIST},
        {"upgrade", no_argument, 0, CONFIG_SUBOP_UPGRADE},
        {"menu", no_argument, 0, CONFIG_SUBOP_MENU},
        {"start", no_argument, 0, CONFIG_SUBOP_START},
        {"close", no_argument, 0, CONFIG_SUBOP_STOP},
        {"cluster", required_argument, 0, OP_CLUSTER},
		{0, 0, 0, 0}
	};

    conf->program_name = strdup(argv[0]);

	while (((c = getopt_long(argc, argv, optstring, opts, &option_index)) != -1)) {
		switch (c) {
			case 0:
				break;
			case '?':
				/* Unknown Option */
				return -EPERM;
			default:
				parseargs_op(c);
				break;
		}
	}

    if (conf->help == true) {
        switch (conf->op) {
            case CONFIG_OP_SERVER:
                usage_server(config_get_program_name());
                break;
            case CONFIG_OP_MOD:
                usage_mod(config_get_program_name());
                break;
            default:
                usage(config_get_program_name());
        }

        return 0;
    }

    if (conf->op == 0) {
        fprintf(stderr, "no operation specified (use -h for help)\n");
        return -EPERM;
    }

	/* parse all other options */
	optind = 1;
	while (((c = getopt_long(argc, argv, optstring, opts, &option_index)) != -1)) {
        switch (conf->op) {
            case CONFIG_OP_SERVER:
                parseargs_server(c);
                break;
            case CONFIG_OP_MOD:
                parseargs_mod(c);
                break;
            default:
                return -EPERM;
        }
	}

    conf->argument_list = argv + optind;
    return 0;
}


inline bool config_is_install_mod()
{
    return conf->op == CONFIG_OP_MOD && conf->sub_op == CONFIG_SUBOP_NONE;
}

inline bool config_is_list_mod()
{
    return conf->op == CONFIG_OP_MOD && conf->sub_op == CONFIG_SUBOP_LIST;
}

inline bool config_is_upgrade_mod()
{
    return conf->op == CONFIG_OP_MOD && conf->sub_op == CONFIG_SUBOP_UPGRADE;
}

inline bool config_is_configure_mod()
{
    return conf->op == CONFIG_OP_MOD && conf->sub_op == CONFIG_SUBOP_MENU;
}

inline bool config_is_start_server()
{
    return conf->op == CONFIG_OP_SERVER && conf->sub_op == CONFIG_SUBOP_START;
}

inline bool config_is_stop_server()
{
    return conf->op == CONFIG_OP_SERVER && conf->sub_op == CONFIG_SUBOP_STOP;
}

inline bool config_is_upgrade_server()
{
    return conf->op == CONFIG_OP_SERVER && conf->sub_op == CONFIG_SUBOP_UPGRADE;
}

inline bool config_is_server_menu()
{
    return conf->op == CONFIG_OP_SERVER && conf->sub_op == CONFIG_SUBOP_MENU;
}

inline const char *config_get_wax_dir()
{
    return conf->wax_dir;
}

inline const char *config_get_dst_app_dir()
{
    return conf->dst_app_dir;
}

inline const char *config_get_dst_cluster_dir()
{
    return conf->dst_cluster_dir;
}

inline const char *config_get_server_binary_dir()
{
    return conf->dst_server_binary_dir;
}

inline const char *config_get_dst_workshop_download_dir()
{
    return conf->dst_workshop_download_dir;
}

inline const char *config_get_cluster_name()
{
    return conf->cluster_name;
}

inline const char *config_get_token_path()
{
    return conf->token_path;
}

inline const char *config_get_config_file_path()
{
    return conf->config_file_path;
}

inline const char *config_get_server_exe_name()
{
    return conf->dst_server_exe_name;
}

inline const char *config_get_program_name()
{
    return conf->program_name;
}

inline const char * const *config_get_arguments_list()
{
    return (const char * const *)conf->argument_list;
}

inline int config_get_arguments_list_len()
{
    int i;

    for (i = 0; conf->argument_list[i] != NULL; ++i);
    return i;
}


const char *config_get_token()
{
    FILE *fp = fopen(config_get_token_path(), "r");

    memset(conf->token, 0, TOKEN_MAXSIZE);

    /* If no such file, try to create one */
    if (fp == NULL)
        fp = fopen(config_get_token_path(), "w+");

    if (fp == NULL) {
        fprintf(stderr, "cannot open token file %s: %s\n", config_get_token_path(), strerror(errno));
        goto ret;
    }

    fread(conf->token, 1, TOKEN_MAXSIZE, fp);
clean:
    fclose(fp);
ret:
    return conf->token;
}


void config_set_token(const char *token)
{
    FILE *fp = fopen(config_get_token_path(), "w+");
    if (fp == NULL) {
        fprintf(stderr, "cannot open token file %s: %s\n", config_get_token_path(), strerror(errno));
        return;
    }

    fwrite(token, 1, strlen(token), fp);
    fclose(fp);
    return;
}


void config_set_cluster_name(const char *value)
{
    free(conf->cluster_name);
    conf->cluster_name = strdup(value);
}


void config_set_value(const char *key, const char *value)
{
    if (strlen(value) == 0)
        return;

    if (strcmp("ClusterName", key) == 0)
        config_set_cluster_name(value);
    else
        fprintf(stderr, "Unknown configuration at \'%s\': %s = %s\n", conf->config_file_path, key, value);

    return;
}


void config_save()
{
    FILE *fp;
    char buff[1024];

    memset(buff, 0, sizeof(buff));
    fp = fopen(conf->config_file_path, "w+");
    if (fp == NULL) {
        fprintf(stderr, "cannot save configuration file %s: %s\n", conf->config_file_path, strerror(errno));
        return;
    }
    
    snprintf(buff, sizeof(buff), "%s = %s\n", "ClusterName", conf->cluster_name);
    fputs(buff, fp);
    fclose(fp);
}


void config_load()
{
    FILE *fp = NULL;

    fp = fopen(conf->config_file_path, "r+");
    if (fp == NULL)
        return;

    yyin = fp;
    yyparse();
    fclose(fp);
}
