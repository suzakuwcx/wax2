#include <stdbool.h>

#include <wax/vector.h>

struct cluster_conf;

enum game_mode {
    GAMEMODE_SURVIVAL = 0,
    GAMEMODE_ENDLESS,
    GAMEMODE_WILDERNESS,
};


/**
 * @brief get current list of all the cluster and push the value into vector
 * 
 * @param vec Vector object, created by 'new_vector()'
 * @return 0  If success
 * @return -1 If error occur
 *
 * @code
 * struct vector *vec = new_vector();
 * cluster_list(vec);
 * for (int i = 0; i < vector_len(vec); ++i)
 *     printf("cluster: %s\n", vector_get(vec, i));
 * @endcode
 */
int cluster_list(struct vector *vec);


/**
 * @brief Create a cluster_conf instances from 'cluster.ini' under cluster, if
 *          no 'cluster.ini' exist, will create an instance with default value
 * 
 * @param name The cluster name under dst cluster directory
 * @return struct cluster_conf* The 'cluster_conf' instances, stand for the
 *                                configuration
 */
struct cluster_conf *new_cluster_conf(const char *name);


/**
 * @brief create a new cluster director 
 * 
 * @param name The cluster n
 * @return 0
 * 
 * @code
 * cluster_create("Cluster_3");
 * @endcode
 */
int cluster_create(const char *name);


/**
 * @brief save the configuration instances to 'cluster.ini', the location of
 *         the 'cluster.ini' is where the file being open at 'new_cluster_conf'
 * 
 * @param conf The cluster_conf instance, created by 'new_cluster_conf'
 * @return 0 If success
 */
int cluster_conf_save(struct cluster_conf *conf);


/**
 * @brief This is the function for lex/yacc, to get current cluster_conf 
 8          instance to modify
 * 
 * @return struct cluster_conf* Current cluster_conf instance, a global static
 *           variable in 'cluster.c'
 */
struct cluster_conf *cluster_get_current_conf();


/**
 * @brief delete the cluster_conf instance and free all the memory
 * 
 * @param conf The cluster_conf instance, created by 'new_cluster_conf'
 *
 * @code
 * struct cluster_conf *conf = new_cluster_conf("cluster_1");
 * cluster_set_max_player(conf, 4);
 * cluster_conf_save(conf);
 * cluster_conf_delete(conf);
 * @endcode
 */
void cluster_conf_delete(struct cluster_conf *conf);


struct vector *cluster_get_gamemode_chosen_list();

int cluster_set_mode(struct cluster_conf *conf, const char *value);
int cluster_set_max_player(struct cluster_conf *conf, const char *value);
int cluster_set_pvp(struct cluster_conf *conf, const char *value);
int cluster_set_vote(struct cluster_conf *conf, const char *value);
int cluster_set_server_name(struct cluster_conf *conf, const char *value);
int cluster_set_server_description(struct cluster_conf *conf, const char *value);
int cluster_set_server_password(struct cluster_conf *conf, const char *value);

int cluster_set_value(struct cluster_conf *conf, const char *key, const char *value);

const char *cluster_get_gamemode(struct cluster_conf *conf);
int cluster_get_max_players(struct cluster_conf *conf);
bool cluster_get_enable_pvp(struct cluster_conf *conf);
bool cluster_get_enable_rollback_vote(struct cluster_conf *conf);
const char *cluster_get_server_name(struct cluster_conf *conf);
const char *cluster_get_server_description(struct cluster_conf *conf);
const char *cluster_get_server_password(struct cluster_conf *conf);
