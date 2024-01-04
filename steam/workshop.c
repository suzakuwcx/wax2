#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <wax/libwax.h>
#include <steam/workshop.h>
#include <wax/conf.h>


extern int errno;


static void __attribute__((constructor)) init_module_curl()
{
    curl_global_init(CURL_GLOBAL_ALL);
}


static void __attribute__((destructor)) module_curl_exit()
{
    curl_global_cleanup();
}


static int curl_download_request_cdn(const char *url, FILE *fp)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init() failed\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    /* Follow redirects */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    return res;
}


static int curl_download_file_request_cdn(const char *url, const char *path)
{
    int ret;
    FILE *fp = fopen(path, "w+");
    ret = curl_download_request_cdn(url, fp);
    fclose(fp);
    return ret;
}


static int curl_post_request(const char *url, const char *post_field, char *recvp, size_t size)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;

    curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "curl_easy_init failed\n");
        return -1;
    }

    fp = fmemopen(recvp, size, "r+");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_field);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    
    curl_easy_cleanup(curl);

    fclose(fp);

    return 0;
}


/*
 * Note: due to that the 'steam_api' need 'steamclient.so' to initialize, but 
 * none of the public download link was found, so use steamcmd as wrapper
 * to access steam workshop. But as you can see, the code is ugly, qwq.
 */ 
static int download_steamcmd(const char *path)
{
    int ret;
    
    FILE *fp = fopen(path, "w+");
    if (fp == NULL) {
        perror("cannot download steamcmd");
        return -1;
    }

    ret = curl_download_request_cdn("https://steamcdn-a.akamaihd.net/client/installer/steamcmd_linux.tar.gz", fp);
    
    fclose(fp);
    return ret;
}


static int steamcmd_install(const char *path)
{
    const char *dl = "/tmp/steamcmd_linux.tar.gz";
    int ret = 0;

    ret = mkdir_p(path);
    if (ret < 0) {
        fprintf(stderr, "Error: cannot create steamcmd install path\n");
        return ret;
    }

    ret = download_steamcmd(dl);
    if (ret < 0) {
        fprintf(stderr, "Error: cannot download %s\n", dl);
        return ret;
    }

    systemf("cd %s && tar -zxvf %s", path, dl);
    remove(dl);

    return ret;
}


static int steamcmd_run_command(const char *argv)
{
    const char *path = config_get_steamcmd_path();
    const char *exe = "steamcmd.sh";
    char buff[128];

    memset(buff, 0, sizeof(buff));

    snprintf(buff, sizeof(buff), "%s/%s", path, exe);

    if (access(buff, F_OK) < 0) {
        fprintf(stderr, "steamcmd.sh not found, start to download\n");
        steamcmd_install(path);
    }

    systemf("cd %s && ./%s +login anonymous %s +quit", path, exe, argv);
    return 0;
}


static int steamcmd_app_update(const char *app_id)
{
    char command[128];

    memset(command, 0, sizeof(command));
    snprintf(command, sizeof(command), "+app_update %s", app_id);
    
    steamcmd_run_command(command);

    return 0;
}


static int steamcmd_workshop_item_downloads(const char * const *ids, size_t len)
{
    char command[4096];
    char buff[128];

    memset(command, 0, sizeof(command));

    for (int i = 0; i < len; ++i) {
        snprintf(buff, sizeof(buff), "+workshop_download_item 322330 %s ", ids[i]);
        strcat(command, buff);
        memset(buff, 0, sizeof(buff));
    }

    steamcmd_run_command(command);

    return 0;
}


static int steamcmd_workshop_item_download(const char *id)
{
    const char *ids[] = {""};
    ids[0] = id;
    return steamcmd_workshop_item_downloads(ids, 1);
}


/**
 * @brief Get the item download url, if the mod is a steam workshop mod,
 *        the return url will be empty 
 * 
 * @param id Workshop id
 * @param download_url The return value for download url
 * @param maxlen The max length of the buffer 'download_url'
 * @return -1 An error occur
 * @return 0 Success and this mod is a ugc mod
 * @return 1 Success but this mod is a steam workshop mod
 */
static int get_workshop_item_download_url(const char *id, char *download_url, size_t maxlen)
{
    int ret;
    char post_field[64];
    char recv_field[4096];
    char *file_url;

    const char *url = "https://db.steamworkshopdownloader.io/prod/api/details/file";

    memset(post_field, 0, sizeof(post_field));
    memset(recv_field, 0, sizeof(recv_field));

    snprintf(post_field, sizeof(post_field), "[%s]", id);

    ret = curl_post_request(url, post_field, recv_field, sizeof(recv_field));
    if (ret < 0) {
        fprintf(stderr, "Cannot access %s", url);
        return -1;
    }

    file_url = strstr(recv_field, "\"file_url\":\"") + 12;
    if (file_url == NULL) {
        fprintf(stderr, "invalid content recv: %s", recv_field);
        return -1;
    }

    /* Note: if this mod is not a ugc mod, the result string will be "file_url":"" */
    if (file_url[0] == '"') 
        return 1;

    file_url = strtok(file_url, "\"");
    strncpy(download_url, file_url, maxlen);
    return 0;
}


int download_workshop(const char *id)
{
    const char *ids[] = {""};
    ids[0] = id;
    download_workshops(ids, 1);
    return 0;
}


int download_workshops(const char * const *ids, int len)
{
    int ret;
    char download_url[1024];
    char workshop_mod_download_location[PATH_MAX];
    char dst_mod_install_location[PATH_MAX];
    struct vector *vec;

    memset(download_url, 0, sizeof(download_url));
    memset(workshop_mod_download_location, 0, sizeof(workshop_mod_download_location));
    vec = new_vector(NULL);

    for (int i = 0; i < len; ++i) {
        ret = get_workshop_item_download_url(ids[i], download_url, sizeof(download_url));
        if (ret < 0){
            fprintf(stderr, "cannot get workshop download url: %s", strerror(errno));
            return ret;
        }

        snprintf(dst_mod_install_location, sizeof(dst_mod_install_location),
                "%s/mods/workshop-%s", config_get_dst_server_path(), ids[i]);

        if (ret == 0) { /* ugc mod */
            curl_download_file_request_cdn(download_url, "/tmp/mod_publish_data_file.zip");
            rm_r(dst_mod_install_location);
            mkdir_p(dst_mod_install_location);
            systemf("cd \"%s\" && unzip \"%s\"\n", dst_mod_install_location, "/tmp/mod_publish_data_file.zip");
            rm_r("/tmp/mod_publish_data_file.zip");
        } else { /* workshop mod */
            vector_push(vec, ids[0], strlen(ids[0]) + 1);
        }
    }

    const char **workshop_ids = calloc(vector_len(vec), sizeof(char *));

    for (int i = 0; i < vector_len(vec); ++i)
        workshop_ids[i] = strdup((char *)vector_get(vec, i));

    steamcmd_workshop_item_downloads(workshop_ids, vector_len(vec));

    for (int i = 0; i < vector_len(vec); ++i) {
        snprintf(dst_mod_install_location, sizeof(dst_mod_install_location),
        "%s/mods/workshop-%s", config_get_dst_server_path(), workshop_ids[i]);
        snprintf(workshop_mod_download_location, sizeof(workshop_mod_download_location), 
                "%s/%s", config_get_dst_workshop_download_path(), workshop_ids[i]);
        mkdir_p(dst_mod_install_location);
        rm_r(dst_mod_install_location);
        ret = mv(workshop_mod_download_location, dst_mod_install_location);
        if (ret < 0)
            fprintf(stderr, "cannot move from %s to %s\n", workshop_mod_download_location, dst_mod_install_location);

        free((char *)workshop_ids[i]);
    }

clean:
    free(workshop_ids);
    vector_delete(vec);
    return 0;
}


inline int download_dst_server()
{
    return steamcmd_app_update("343050");    
}
