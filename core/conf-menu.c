#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <wax/conf.h>
#include <wax/dialog.h>
#include <wax/libwax.h>
#include <wax/vector.h>
#include <steam/cluster.h>

static const char mconf_readme[] =
"Overview\n"
"--------\n"
"This interface lets you select features and parameters for the build.\n"
"Features can either be built-in, modularized, or ignored. Parameters\n"
"must be entered in as decimal or hexadecimal numbers or text.\n"
"\n"
"Menu items beginning with following braces represent features that\n"
"  [ ] can be built in or removed\n"
"  < > can be built in, modularized or removed\n"
"  { } can be built in or modularized (selected by other feature)\n"
"  - - are selected by other feature,\n"
"while *, M or whitespace inside braces means to build in, build as\n"
"a module or to exclude the feature respectively.\n"
"\n"
"To change any of these features, highlight it with the cursor\n"
"keys and press <Y> to build it in, <M> to make it a module or\n"
"<N> to remove it.  You may also press the <Space Bar> to cycle\n"
"through the available options (i.e. Y->N->M->Y).\n"
"\n"
"Some additional keyboard hints:\n"
"\n"
"Menus\n"
"----------\n"
"o  Use the Up/Down arrow keys (cursor keys) to highlight the item you\n"
"   wish to change or the submenu you wish to select and press <Enter>.\n"
"   Submenus are designated by \"--->\", empty ones by \"----\".\n"
"\n"
"   Shortcut: Press the option's highlighted letter (hotkey).\n"
"             Pressing a hotkey more than once will sequence\n"
"             through all visible items which use that hotkey.\n"
"\n"
"   You may also use the <PAGE UP> and <PAGE DOWN> keys to scroll\n"
"   unseen options into view.\n"
"\n"
"o  To exit a menu use the cursor keys to highlight the <Exit> button\n"
"   and press <ENTER>.\n"
"\n"
"   Shortcut: Press <ESC><ESC> or <E> or <X> if there is no hotkey\n"
"             using those letters.  You may press a single <ESC>, but\n"
"             there is a delayed response which you may find annoying.\n"
"\n"
"   Also, the <TAB> and cursor keys will cycle between <Select>,\n"
"   <Exit>, <Help>, <Save>, and <Load>.\n"
"\n"
"o  To get help with an item, use the cursor keys to highlight <Help>\n"
"   and press <ENTER>.\n"
"\n"
"   Shortcut: Press <H> or <?>.\n"
"\n"
"o  To toggle the display of hidden options, press <Z>.\n"
"\n"
"\n"
"Radiolists  (Choice lists)\n"
"-----------\n"
"o  Use the cursor keys to select the option you wish to set and press\n"
"   <S> or the <SPACE BAR>.\n"
"\n"
"   Shortcut: Press the first letter of the option you wish to set then\n"
"             press <S> or <SPACE BAR>.\n"
"\n"
"o  To see available help for the item, use the cursor keys to highlight\n"
"   <Help> and Press <ENTER>.\n"
"\n"
"   Shortcut: Press <H> or <?>.\n"
"\n"
"   Also, the <TAB> and cursor keys will cycle between <Select> and\n"
"   <Help>\n"
"\n"
"\n"
"Data Entry\n"
"-----------\n"
"o  Enter the requested information and press <ENTER>\n"
"   If you are entering hexadecimal values, it is not necessary to\n"
"   add the '0x' prefix to the entry.\n"
"\n"
"o  For help, use the <TAB> or cursor keys to highlight the help option\n"
"   and press <ENTER>.  You can try <TAB><H> as well.\n"
"\n"
"\n"
"Text Box    (Help Window)\n"
"--------\n"
"o  Use the cursor keys to scroll up/down/left/right.  The VI editor\n"
"   keys h,j,k,l function here as do <u>, <d>, <SPACE BAR> and <B> for\n"
"   those who are familiar with less and lynx.\n"
"\n"
"o  Press <E>, <X>, <q>, <Enter> or <Esc><Esc> to exit.\n"
"\n"
"\n"
"Alternate Configuration Files\n"
"-----------------------------\n"
"Menuconfig supports the use of alternate configuration files for\n"
"those who, for various reasons, find it necessary to switch\n"
"between different configurations.\n"
"\n"
"The <Save> button will let you save the current configuration to\n"
"a file of your choosing.  Use the <Load> button to load a previously\n"
"saved alternate configuration.\n"
"\n"
"Even if you don't use alternate configuration files, but you find\n"
"during a Menuconfig session that you have completely messed up your\n"
"settings, you may use the <Load> button to restore your previously\n"
"saved settings from \".config\" without restarting Menuconfig.\n"
"\n"
"Other information\n"
"-----------------\n"
"If you use Menuconfig in an XTERM window, make sure you have your\n"
"$TERM variable set to point to an xterm definition which supports\n"
"color.  Otherwise, Menuconfig will look rather bad.  Menuconfig will\n"
"not display correctly in an RXVT window because rxvt displays only one\n"
"intensity of color, bright.\n"
"\n"
"Menuconfig will display larger menus on screens or xterms which are\n"
"set to display more than the standard 25 row by 80 column geometry.\n"
"In order for this to work, the \"stty size\" command must be able to\n"
"display the screen's current row and column geometry.  I STRONGLY\n"
"RECOMMEND that you make sure you do NOT have the shell variables\n"
"LINES and COLUMNS exported into your environment.  Some distributions\n"
"export those variables via /etc/profile.  Some ncurses programs can\n"
"become confused when those variables (LINES & COLUMNS) don't reflect\n"
"the true screen size.\n"
"\n"
"Optional personality available\n"
"------------------------------\n"
"If you prefer to have all of the options listed in a single menu,\n"
"rather than the default multimenu hierarchy, run the menuconfig with\n"
"MENUCONFIG_MODE environment variable set to single_menu. Example:\n"
"\n"
"make MENUCONFIG_MODE=single_menu menuconfig\n"
"\n"
"<Enter> will then unroll the appropriate category, or enfold it if it\n"
"is already unrolled.\n"
"\n"
"Note that this mode can eventually be a little more CPU expensive\n"
"(especially with a larger number of unrolled categories) than the\n"
"default mode.\n"
"\n"

"Search\n"
"-------\n"
"Pressing the forward-slash (/) anywhere brings up a search dialog box.\n"
"\n"

"Different color themes available\n"
"--------------------------------\n"
"It is possible to select different color themes using the variable\n"
"MENUCONFIG_COLOR. To select a theme use:\n"
"\n"
"make MENUCONFIG_COLOR=<theme> menuconfig\n"
"\n"
"Available themes are\n"
" mono       => selects colors suitable for monochrome displays\n"
" blackbg    => selects a color scheme with black background\n"
" classic    => theme with blue background. The classic look\n"
" bluetitle  => an LCD friendly version of classic. (default)\n"
"\n",
menu_instructions[] =
	"Arrow keys navigate the menu.  "
	"<Enter> selects submenus ---> (or empty submenus ----).  "
	"Highlighted letters are hotkeys.  "
	"Pressing <Y> includes, <N> excludes, <M> modularizes features.  "
	"Press <Esc><Esc> to exit, <?> for Help, </> for Search.  "
	"Legend: [*] built-in  [ ] excluded  <M> module  < > module capable",
radiolist_instructions[] =
	"Use the arrow keys to navigate this window or "
	"press the hotkey of the item you wish to select "
	"followed by the <SPACE BAR>. "
	"Press <?> for additional information about this option.",
inputbox_instructions_int[] =
	"Please enter a decimal value. "
	"Fractions will not be accepted.  "
	"Use the <TAB> key to move from the input field to the buttons below it.",
inputbox_instructions_hex[] =
	"Please enter a hexadecimal value. "
	"Use the <TAB> key to move from the input field to the buttons below it.",
inputbox_instructions_string[] =
	"Please enter a string value. "
	"Use the <TAB> key to move from the input field to the buttons below it.",
setmod_text[] =
	"This feature depends on another which has been configured as a module.\n"
	"As a result, this feature will be built as a module.",
load_config_text[] =
	"Enter the name of the configuration file you wish to load.  "
	"Accept the name shown to restore the configuration you "
	"last retrieved.  Leave blank to abort.",
load_config_help[] =
	"\n"
	"For various reasons, one may wish to keep several different\n"
	"configurations available on a single machine.\n"
	"\n"
	"If you have saved a previous configuration in a file other than the\n"
	"default one, entering its name here will allow you to modify that\n"
	"configuration.\n"
	"\n"
	"If you are uncertain, then you have probably never used alternate\n"
	"configuration files. You should therefore leave this blank to abort.\n",
save_config_text[] =
	"Enter a filename to which this configuration should be saved "
	"as an alternate.  Leave blank to abort.",
save_config_help[] =
	"\n"
	"For various reasons, one may wish to keep different configurations\n"
	"available on a single machine.\n"
	"\n"
	"Entering a file name here will allow you to later retrieve, modify\n"
	"and use the current configuration as an alternate to whatever\n"
	"configuration options you have selected at that time.\n"
	"\n"
	"If you are uncertain what all this means then you should probably\n"
	"leave this blank.\n",
search_help[] =
	"\n"
	"Search for symbols and display their relations.\n"
	"Regular expressions are allowed.\n"
	"Example: search for \"^FOO\"\n"
	"Result:\n"
	"-----------------------------------------------------------------\n"
	"Symbol: FOO [=m]\n"
	"Type  : tristate\n"
	"Prompt: Foo bus is used to drive the bar HW\n"
	"  Location:\n"
	"    -> Bus options (PCI, PCMCIA, EISA, ISA)\n"
	"      -> PCI support (PCI [=y])\n"
	"(1)     -> PCI access mode (<choice> [=y])\n"
	"  Defined at drivers/pci/Kconfig:47\n"
	"  Depends on: X86_LOCAL_APIC && X86_IO_APIC || IA64\n"
	"  Selects: LIBCRC32\n"
	"  Selected by: BAR [=n]\n"
	"-----------------------------------------------------------------\n"
	"o The line 'Type:' shows the type of the configuration option for\n"
	"  this symbol (bool, tristate, string, ...)\n"
	"o The line 'Prompt:' shows the text used in the menu structure for\n"
	"  this symbol\n"
	"o The 'Defined at' line tells at what file / line number the symbol\n"
	"  is defined\n"
	"o The 'Depends on:' line tells what symbols need to be defined for\n"
	"  this symbol to be visible in the menu (selectable)\n"
	"o The 'Location:' lines tells where in the menu structure this symbol\n"
	"  is located\n"
	"    A location followed by a [=y] indicates that this is a\n"
	"    selectable menu item - and the current value is displayed inside\n"
	"    brackets.\n"
	"    Press the key in the (#) prefix to jump directly to that\n"
	"    location. You will be returned to the current search results\n"
	"    after exiting this new menu.\n"
	"o The 'Selects:' line tells what symbols will be automatically\n"
	"  selected if this symbol is selected (y or m)\n"
	"o The 'Selected by' line tells what symbol has selected this symbol\n"
	"\n"
	"Only relevant lines are shown.\n"
	"\n\n"
	"Search examples:\n"
	"Examples: USB	=> find all symbols containing USB\n"
	"          ^USB => find all symbols starting with USB\n"
	"          USB$ => find all symbols ending with USB\n"
	"\n";


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
    dialog_inputbox(title, inputbox_instructions_string, 10, 75, NULL);

    strcpy(buf, dialog_input_result);
	return 0;
}


int conf_menu()
{
    int res = 0;

    int s_scroll = 0;
    int selected = 0;
    char token[TOKEN_MAXSIZE];
	char cluster[FILENAME_MAX];
	char buff[256];
	struct vector *vec = new_vector(NULL);

	memset(buff, 0, sizeof(buff));

	strncpy(cluster, config_get_cluster_name(), sizeof(cluster));
    strncpy(token, config_get_token(), sizeof(token));
	cluster_list(vec);

    if (init_dialog(NULL)) {
		fprintf(stderr, "Your display is too small to run Menuconfig!\n");
		fprintf(stderr, "It must be at least 19 lines by 80 columns.\n");
		return -EPERM;
	}
    
    while (1) {
        item_reset();

        item_make("(%s) Token", token);
		item_make("Cluster (%s)  -->", cluster);
		item_make("---");
		item_make("Create New Cluster");
		item_set_selected(selected);
        dialog_clear();

        res = dialog_menu("Configuration",
                    menu_instructions,
                    NULL, &s_scroll);


		/* 
		 * Enter, Select "Exit"
		 * ESC Key
		 * Terminal resize and become too small
		 */
		if (res == 1 || res == KEY_ESC || res == -ERRDISPLAYTOOSMALL)
			break;

        selected = item_n();

		switch (res) {
			case 0: /* Enter, Select "Select" */
				if (selected == 0) {
					input_box("Token", token);
				}
				else if (selected == 1)  {
					check_list("Cluster", vec, vector_find(vec, cluster, 0));
					if (vector_len(vec) != 0)
						strncpy(cluster, vector_get(vec, res), FILENAME_MAX);
				}
				else if (selected == 3) {
					input_box("Cluster Name", buff);
					cluster_create(buff);
					vector_delete(vec);
					vec = new_vector(NULL);
					cluster_list(vec);
					text_box("Configuration", "Cluster create");
				}
				break;
			case 3: /* Enter, Select "Save" */
				config_set_token(token);
				config_set_cluster_name(cluster);
				config_save();
				text_box("Configuration", "Configuration save");
				break;
		}
    }

    end_dialog(saved_x, saved_y);
	vector_delete(vec);
    return selected;
}
