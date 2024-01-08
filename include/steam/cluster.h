#include <wax/vector.h>


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
 * @brief create a new cluster 
 * 
 * @param name The cluster name
 * @return 0  If success
 * @return -1 If error occur
 * 
 * @code
 * cluster_create("Cluster_3");
 * @endcode
 */
int cluster_create(const char *name);
