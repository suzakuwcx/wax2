#include <stdbool.h>

struct config;

enum wax_op {
    CONFIG_OP_NONE = 0,
    CONFIG_OP_MOD,
};

enum wax_subop {
    CONFIG_SUBOP_NONE = 0,
    CONFIG_SUBOP_LIST,
    CONFIG_SUBOP_UPGRADE,
    CONFIG_SUBOP_MENU,
};

int parseargs(int argc, char *argv[]);

bool config_is_install_mod();
bool config_is_list_mod();
bool config_is_upgrade_mod();
bool config_is_configure_mod();
const char *config_get_steamcmd_path();
const char *config_get_dst_server_path();
const char *config_get_dst_workshop_download_path();
const char *config_get_program_name();
const char * const *config_get_arguments_list();
int config_get_arguments_list_len();
