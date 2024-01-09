#ifndef _LIBWAX_H_
#define _LIBWAX_H_ 1

#include <stdbool.h>
#include <stddef.h>


/**
 * @brief Run command with format string
 * 
 * @param format The format string, same as 'printf'
 * @param ... The argument list
 * @return int The return value of 'system()' function
 *
 * @code
 * systemf("ls %s", "/tmp");
 * @endcode
 * 
 */
int systemf(const char *restrict format, ...);


/**
 * @brief Create directory like 'mkdir -p <path>'
 * 
 * @param path The target path to be created
 * @return 0 If everything ok
 * @return 1 If error occur
 * 
 * @code
 * // create directory /a/b/c/d/e/f
 * ret = mkdir_p("a/b/c/d/e/f");
 * if (ret < 0)
 *     fprintf(stderr, "create directory failed\n");
 * @endcode 
 */
int mkdir_p(const char *path);


/**
 * @brief Move directory like 'mv <old> <new>'
 * 
 * @param old Old path
 * @param new New path
 * @return 0 If everything ok
 * @return 1 If error occur
 *
 * @code
 * ret = mv("dir", "/tmp");
 * if (ret < 0)
 *     fprintf(stderr, "move directory failed\n");
 * @endcode
 */
int mv(const char *old, const char *new);


/**
 * @brief Copy file like 'cp -r <src> <dst>'
 * 
 * @param src The source file location
 * @param dst The destination file location
 * @return 0 If everything ok
 * @return 1 If error occur
 *
 * @code
 * ret = cp_r("dir", "/tmp");
 * if (ret < 0)
 *     fprintf(stderr, "move directory failed\n");
 * @endcode
 */
int cp_r(const char *src, const char *dst);


/**
 * @brief Remove file like 'rm -r <path>'
 * 
 * @param path The file that will be remove recursion
 * @return 0 If everything ok
 * @return 1 If error occur
 * @code
 * ret = cp_r("dir", "/tmp");
 * if (ret < 0)
 *     fprintf(stderr, "move directory failed\n");
 * @endcode
 */
int rm_r(const char *path);


/**
 * @brief Test if a C string a valid natural number
 * 
 * @param str A c string to be test
 * @return true This string is natural number
 * @return false This string is not an natural number
 * @code
 * is_string_number("12345"); // return true
 * is_string_number("-1345"); // return false
 * is_string_number("0x1A"); // return false
 * @endcode
 */
bool is_string_number(const char *str);


/**
 * @brief check if this string 'true' or 'false'
 * 
 * @param str The string to be test
 * @return true The string is bool
 * @return false The string is not a bool
 */
bool is_string_bool(const char *str);


/**
 * @brief Convert string 'true' or 'false' to bool
 * 
 * @param str 'true' or 'false'
 * @return true Is string 'true'
 * @return false Is string 'false'
 */
bool string_to_bool(const char *str);


struct dfa;


struct dfa *new_dfa(char *regex);
int dfa_check(struct dfa *d, char *buf, size_t len);
void dfa_reset(struct dfa *d);
void dfa_delete(struct dfa *d);


void clear_screen();
void clear_line();


#endif /* _LIBWAX_H_ */
