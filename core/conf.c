/**
 * @cite https://gitlab.archlinux.org/pacman/pacman.git 18e49f2c97f0e33a645f364ed9de8e3da6c36d41 pacman.c:parseargs
 */

#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>

#include <wax/conf.h>

extern int errno;

struct config {
    char *program_name;
    char *steam_path;
    char *wax_path;
    char *dst_server_path;
    char *dst_workshop_download_path;

    char * const *argument_list;

    enum wax_op op;
    enum wax_subop sub_op;
    bool help;

    bool op_m_menu;
};

static struct config *conf;

static void __attribute__((constructor)) init_module_conf() 
{
    char buff[256];
    char *p;

    memset(buff, 0, sizeof(buff));

    conf = calloc(1, sizeof(struct config));
    if (conf == NULL) {
        perror("Cannot assign memory for config");
        exit(1);
    }

    p = getenv("HOME");
    if (p == NULL) {
        fprintf(stderr, "cannot get environment variable HOME\n");
        exit(1);
    }

    // snprintf(buff, sizeof(buff), "%s/.local/share/wax/", p);
    snprintf(buff, sizeof(buff), "%s/.local1/share/wax/", p);
    conf->wax_path = strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/Steam/", p);
    conf->steam_path = strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/steamapps/common/Don't Starve Together Dedicated Server/", conf->steam_path);
    conf->dst_server_path = strdup(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s/steamapps/workshop/content/322330/", conf->steam_path);
    conf->dst_workshop_download_path = strdup(buff);

    return;
}


static void __attribute__((destructor)) module_conf_exit()
{
    free(conf->dst_workshop_download_path);
    free(conf->dst_server_path);
    free(conf->steam_path);
    free(conf->wax_path);

    if (conf->program_name != NULL) free(conf->program_name);
    free(conf);
} 


static void usage(const char *name)
{
	printf("Usage:  %s <operation> [...]\n", name);
	printf("Operations: \n");
	printf("    {-M --mod} <options>\n");
	printf("\n");
	printf("Examples:\n");
	printf("  -M 1242093526          install Mod via workshop id\n");
	printf("\n");
	printf("Use '%s {-h --help}' with an operationfor available options\n", name);
    printf("\n");
	printf("Config file location is '%s' and '%s/Steam'\n", config_get_steamcmd_path(), getenv("HOME"));
}


static void usage_mod(const char *name)
{
	printf("Usage:  %s {-M --mod} [options] [id(s)]\n", name);
	printf("options: \n");
	printf("  -l, --list             list installed mod\n");
    printf("  -u, --upgrade          upgrade all mod\n");
    printf("  -m, --menu             open a tui menu to configure mod configure\n");
	printf("\n");
}


static int parseargs_op(int opt)
{
    switch (opt) {
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
        default:
            return -EPERM;
    }
    
    return 0;
}


int parseargs(int argc, char *argv[])
{
	int c;
	int option_index = 0;

	const char *optstring = "Mhlum";

	static const struct option opts[] =
	{
		// {"server", no_argument, 0, 'S'},
		{"mod", no_argument, 0, 'M'},
		{"help", no_argument, 0, 'h'},

        {"list", no_argument, 0, CONFIG_SUBOP_LIST},
        {"upgrade", no_argument, 0, CONFIG_SUBOP_UPGRADE},
        {"menu", no_argument, 0, CONFIG_SUBOP_MENU},
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

inline const char *config_get_steamcmd_path()
{
    return conf->wax_path;
}

inline const char *config_get_dst_server_path()
{
    return conf->dst_server_path;
}

inline const char *config_get_dst_workshop_download_path()
{
    return conf->dst_workshop_download_path;
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
