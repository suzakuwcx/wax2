#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <steam/workshop.h>
#include <wax/conf.h>
#include <wax/libwax.h>


int main(int argc, char *argv[])
{
	int ret;

	ret = parseargs(argc, argv);
	if (ret < 0)
		return ret;

	if (config_is_install_mod()) {
		int len = config_get_arguments_list_len();
		const char * const *p = config_get_arguments_list();
		for (int i = 0; i < len; ++i) {
			if (!is_string_number(p[i])) {
				fprintf(stderr, "Invalid argument: %s\n", p[i]);
				return -EPERM;
			}
		}

		download_workshops(p, len);
	}

    return 0;
}
