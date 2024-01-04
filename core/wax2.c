#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

#include <steam/workshop.h>
#include <wax/conf.h>
#include <wax/libwax.h>
#include <wax/dtach.h>


static int on_install_mod()
{
	int len = config_get_arguments_list_len();
	const char * const *p = config_get_arguments_list();
	for (int i = 0; i < len; ++i) {
		if (!is_string_number(p[i])) {
			fprintf(stderr, "Invalid argument: %s\n", p[i]);
			return -EPERM;
		}
	}

	download_workshops(p, len);
	return 0;
}


static int on_upgrade_server()
{
	return download_dst_server();
}


static int on_start_server()
{
	int ret = 0;
	char *cluster = strdup(config_get_cluster());
	char *argv_master[] = {"./dontstarve_dedicated_server_nullrenderer_x64", "-shared", "Master", "-cluster", cluster, NULL};
	char *argv_cave[] = {"./dontstarve_dedicated_server_nullrenderer_x64", "-shared", "Caves", "-cluster", cluster, NULL};
	int is_master_success = 0;
	int is_caves_success = 0;

	if (tcgetattr(0, &orig_term) < 0)
	{
		memset(&orig_term, 0, sizeof(struct termios));
		dont_have_tty = 1;
	}

	if (dont_have_tty)
	{
		printf("%s: Attaching to a session requires a terminal.\n",
			progname);
		return 1;
	}

	ret = chdir(config_get_server_binary_path());
	if (ret < 0) {
		fprintf(stderr, "cannot change to server directory %s: %s\n", config_get_server_binary_path(), strerror(errno));
		fprintf(stderr, "note: you can use '%s -Su' to download server binary\n", config_get_program_name());
		ret = -1;
		goto clean;
	}

	set_sockname_master();
    if (attach_main(1, 0) == 1)
    {
        if (errno == ECONNREFUSED || errno == ENOENT)
        {
            if (errno == ECONNREFUSED)
                unlink(sockname);
            if (master_main(argv_master, 1, 0) != 0) {
				ret = 1;
				goto clean;
			}
        }
        ret = attach_main(0, 1);
		if  (ret == 3)
			is_master_success = 1;
    }

	set_sockname_caves();
    if (attach_main(1, 0) != 0)
    {
        if (errno == ECONNREFUSED || errno == ENOENT)
        {
            if (errno == ECONNREFUSED)
                unlink(sockname);
            if (master_main(argv_cave, 1, 0) != 0) {
				ret = 1;
				goto clean;
			}
        }
        ret = attach_main(0, 1);
		if (ret == 3)
			is_caves_success = 1;
    }


clean:
	if (is_master_success) puts("[Master start success]");
	if (is_caves_success) puts("[Caves start success]"); 
	free(cluster);
	return ret;
}


static int on_stop_server()
{
	set_sockname_master();
	push_main("c_shutdown()\n");
	set_sockname_caves();
	push_main("c_shutdown()\n");
	/*
	 * because dst server will not send EOF character even being shutdown, so
	 * send an additional newline character to shutdown
	 */
	sleep(1);
	push_main("\n");
	set_sockname_master();
	sleep(1);
	push_main("\n");
	return 0;
}


int main(int argc, char *argv[])
{
	int ret;

	ret = parseargs(argc, argv);
	if (ret < 0)
		return ret;

	if (config_is_install_mod()) {
		on_install_mod();
	} else if (config_is_upgrade_server()) {
		on_upgrade_server();
	} else if (config_is_start_server()) {
		on_start_server();
	} else if (config_is_stop_server()) {
		on_stop_server();
	}

    return 0;
}
