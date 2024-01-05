#include <stdbool.h>

#define TOKEN_MAXSIZE 256

struct config;

enum wax_op {
    CONFIG_OP_NONE = 0,
    CONFIG_OP_SERVER,
    CONFIG_OP_MOD,
};

enum wax_subop {
    CONFIG_SUBOP_NONE = 0,
    CONFIG_SUBOP_LIST,
    CONFIG_SUBOP_UPGRADE,
    CONFIG_SUBOP_MENU,
    CONFIG_SUBOP_START,
    CONFIG_SUBOP_STOP,
};

enum {
    OP_CLUSTER,
};

int parseargs(int argc, char *argv[]);

bool config_is_install_mod();
bool config_is_list_mod();
bool config_is_upgrade_mod();
bool config_is_configure_mod();
bool config_is_start_server();
bool config_is_stop_server();
bool config_is_upgrade_server();

const char *config_get_wax_dir();
const char *config_get_dst_app_dir();
const char *config_get_cluster_dir();
const char *config_get_server_binary_dir();
const char *config_get_dst_workshop_download_dir();

const char *config_get_token_path();

const char *config_get_token();
const char *config_get_cluster_name();
const char *config_get_program_name();
const char *config_get_server_exe_name();

const char * const *config_get_arguments_list();
int config_get_arguments_list_len();

void config_set_token(const char *token);
