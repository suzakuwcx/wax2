#ifndef _WORKSHOP_H_
#define _WORKSHOP_H_ 1

#include <stdbool.h>
#include <stddef.h>


/**
 * @brief Download single workshop with workshop id
 * 
 * @param id Mod id in workshop
 * @return 0 If success
 * @return 1 if error occur
 *
 * @code
 * download_workshop("1242093526");
 * download_workshop("1392778117");
 * @endcode
 */
int download_workshop(const char *id);


/**
 * @brief Download single workshop with a bunch of workshop id
 *        This will faster than calling 'download_workshop()' each time
 *
 * @param ids Mod list id in workshop
 * @return 0 If success
 * @return 1 If error occur
 * 
 * @code
 * download_workshop({"1242093526", "1392778117"}, 2);
 * @endcode
 */
int download_workshops(const char * const *ids, int len);


/**
 * @brief Download dst server runtime binary
 * 
 * @return 0 If success
 * @return 1 If error occur
 *
 * @code
 * download_dst_server();
 * @endcode
 */
int download_dst_server();

#endif /* _WORKSHOP_H_ */
