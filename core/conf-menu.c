#include <linux/limits.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <wax/conf.h>
#include <wax/dialog.h>
#include <wax/libwax.h>
#include <wax/vector.h>
#include <wax/tui.h>

#include <steam/cluster.h>

struct symbol {
    char token[TOKEN_MAXSIZE];
	char cluster[FILENAME_MAX];
	struct cluster_conf *conf;
};


/*
 * generate a check list tui by items in vectors, return the item being chosen,
 * 'selected' is the first item to be selected in tui
 * if nothing is being chosen, return the 'selected'
 */
static int check_list(const char *title, struct vector *items, int selected)
{
	int res = 0;

	item_reset();

	for (int i = 0; i < vector_len(items); ++i) {
		item_make("%s", vector_get(items, i));
		if (i == selected) item_set_tag('X');
	}

	res = dialog_checklist(title,
		radiolist_instructions,
		MENUBOX_HEIGTH_MIN,
		MENUBOX_WIDTH_MIN,
		CHECKLIST_HEIGTH_MIN);

	if (res == KEY_ESC || res == -ERRDISPLAYTOOSMALL)
		return selected;

	return item_n();
}


static int text_box(const char *title, const char *prompt)
{
	int line = 1;

	/* Test how many line in prompt */
	char *buf = strdup(prompt);
	strtok(buf, "\n");
	while (strtok(NULL, "\n") != NULL)
		++line;

	free(buf);
	
	return dialog_textbox(title, prompt, 5 * line, 75, NULL, NULL,
			      NULL, NULL);
}


static int input_box(const char *title, char *buf)
{
	int res = 0;

    res = dialog_inputbox(title, inputbox_instructions_string, 10, 75, buf);

	if (res == KEY_ESC || res == -ERRDISPLAYTOOSMALL)
		return -1;

    strcpy(buf, dialog_input_result);
	return 0;
}


int conf_menu()
{
    int res = 0;
    int s_scroll = 0;
    int selected = 0;

	char tmp[PATH_MAX];

	char *data;
	struct vector *vec;
	struct vector *gamemode_list;

	struct symbol sym;

    if (init_dialog(NULL)) {
		fprintf(stderr, "Your display is too small to run Menuconfig!\n");
		fprintf(stderr, "It must be at least 19 lines by 80 columns.\n");
		return -EPERM;
	}

	memset(tmp, 0, sizeof(tmp));
	memset(&sym, 0, sizeof(sym));

	strncpy(sym.cluster, config_get_cluster_name(), sizeof(sym.cluster));
    strncpy(sym.token, config_get_token(), sizeof(sym.token));

	gamemode_list = cluster_get_gamemode_chosen_list();
	cluster_create(sym.cluster);
	sym.conf = new_cluster_conf(sym.cluster);
	if (sym.conf == NULL) {
		fprintf(stderr, "cannot create cluster %s\n", sym.cluster);
		return -1;
	}

	vec = new_vector(NULL);
	cluster_list(vec);

    while (1) {
        item_reset();

        item_make("(%s) Token", sym.token);
		item_set_data("token");

		item_make("Cluster (%s)  -->", sym.cluster);
		item_set_data("cluser");

		item_make("  Gamemode: (%s)  --->", cluster_get_gamemode(sym.conf));
		item_set_data("gamemode");

		item_make("  Max players (%d)", cluster_get_max_players(sym.conf));
		item_set_data("players");

		item_make("  [%c] Enable pvp", cluster_get_enable_pvp(sym.conf) ? '*' : ' ');
		item_set_data("pvp");

		item_make("  [%c] Enable rollback vote", cluster_get_enable_rollback_vote(sym.conf) ? '*' : ' ');
		item_set_data("vote");

		item_make("  (%s) Server name", cluster_get_server_name(sym.conf));
		item_set_data("server_name");

		item_make("  (%s) Server description", cluster_get_server_description(sym.conf));
		item_set_data("server_description");

		item_make("  (%s) Server password", cluster_get_server_password(sym.conf));
		item_set_data("server_password");

		item_make("---");
		item_set_data("delimiter");
		
		item_make("Create New Cluster");
		item_set_data("new_cluster");

        dialog_clear();

        res = dialog_menu("Configuration",
                    menu_instructions,
                    data, &s_scroll);

		/* 
		 * Enter, Select "Exit"
		 * ESC Key
		 * Terminal resize and become too small
		 */
		if (res == 1 || res == KEY_ESC || res == -ERRDISPLAYTOOSMALL)
			break;

        selected = item_n();
		data = item_data();

		switch (res) {
		case 0: /* Enter, Select "Select" */
			switch (selected) {
			case 0:
				input_box("Token", sym.token);
				break;
			case 1:
				res = check_list("Cluster", vec, vector_find(vec, sym.cluster, 0));
				if (vector_len(vec) != 0) {
					strncpy(sym.cluster, vector_get(vec, res), FILENAME_MAX);
					cluster_conf_delete(sym.conf);
					sym.conf = new_cluster_conf(sym.cluster);
				}
				break;
			case 2: /* Begin cluster setting */
				res = check_list("Game mode", gamemode_list, vector_find(gamemode_list, cluster_get_gamemode(sym.conf), 0));
				cluster_set_mode(sym.conf, vector_get(gamemode_list, res));
				break;
			case 3:
				snprintf(tmp, sizeof(tmp), "%d", cluster_get_max_players(sym.conf));
				res = input_box("Max player", tmp);
				if (res == 0 && is_string_number(tmp))
					cluster_set_max_player(sym.conf, tmp);
				break;
			case 6:
				snprintf(tmp, sizeof(tmp), "%s", cluster_get_server_name(sym.conf));
				res = input_box("Server name", tmp);
				if (res == 0)
					cluster_set_server_name(sym.conf, tmp);
				break;
			case 7:
				snprintf(tmp, sizeof(tmp), "%s", cluster_get_server_description(sym.conf));
				res = input_box("Server description", tmp);
				if (res == 0)
					cluster_set_server_description(sym.conf, tmp);
				break;
			case 8:
				snprintf(tmp, sizeof(tmp), "%s", cluster_get_server_password(sym.conf));
				res = input_box("Server password", tmp);
				if (res == 0)
					cluster_set_server_password(sym.conf, tmp);
				break;				
			case 10:
				memset(tmp, 0, sizeof(tmp));
				res = input_box("Cluster Name", tmp);
				if (res < 0)
					break;
				
				cluster_create(tmp);
				vector_delete(vec);
				vec = new_vector(NULL);
				cluster_list(vec);
				text_box("Configuration", "Cluster create");			
				break;
			}
			break;
		case 3: /* Enter, Select "Save" */
			config_set_token(sym.token);
			config_set_cluster_name(sym.cluster);
			cluster_conf_save(sym.conf);
			config_save();
			text_box("Configuration", "Configuration save");
			break;
		case 8: /* Space */
			switch (selected) {
			case 4: /* Enable pvp */
				if (cluster_get_enable_pvp(sym.conf))
					cluster_set_pvp(sym.conf, "false");
				else
					cluster_set_pvp(sym.conf, "true");
				break;
			case 5: /* Enable rollback vote */
				if (cluster_get_enable_rollback_vote(sym.conf))
					cluster_set_vote(sym.conf, "false");
				else
					cluster_set_vote(sym.conf, "true");
				break;
			}

			break;
		}
    }

    end_dialog(saved_x, saved_y);
	vector_delete(vec);
    return selected;
}
