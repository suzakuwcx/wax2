#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wax/libwax.h>

#include <steam/world.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

struct world_conf {
    lua_State *L;
    bool is_master;
    char path[PATH_MAX];
};


struct world_conf *new_world_conf(const char *path, bool is_master)
{
    struct world_conf *conf = calloc(1, sizeof(struct world_conf));
    
    char *buff;

    conf->is_master = is_master;
    conf->L = luaL_newstate();
    luaL_openlibs(conf->L);
    luaL_dostring(conf->L, data_dumper);
    luaL_dostring(conf->L, table_merger);

    if (is_master)
        luaL_dostring(conf->L, leveldataoverride_master);
    else
        luaL_dostring(conf->L, leveldataoverride_caves);

    /* place the return table of leveldataoverride to global */ 
    lua_setglobal(conf->L, "data");
    strncpy(conf->path, path, sizeof(conf->path));

    /* merge the default configuration and current configuration */
    buff = fread_a(conf->path);
    if (buff == NULL)
        goto fin;

    lua_getglobal(conf->L, "TableMerger");
    lua_getglobal(conf->L, "data");
    luaL_dostring(conf->L, buff);

    lua_pcall(conf->L, 2, 1, 0);
    lua_setglobal(conf->L, "data");

    free(buff);
fin:
    return conf;
}

void world_conf_save(struct world_conf *conf)
{
    lua_State *L = conf->L;
    const char *dump;
    FILE *fp;

    lua_getglobal(L, "DataDumper");
    lua_getglobal(L, "data");

    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushnil(L);

    lua_pcall(L, 4, 1, 0);

    dump = lua_tostring(L, -1);

    fp = fopen(conf->path, "w+");
    fprintf(fp, "%s", dump);
    fclose(fp);

    lua_pop(L, 1);
}


void world_conf_delete(struct world_conf *conf)
{
    lua_close(conf->L);
    free(conf);
}
