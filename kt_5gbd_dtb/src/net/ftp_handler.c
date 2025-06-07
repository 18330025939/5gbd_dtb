#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <curl/curl.h>
#include "spdlog_c.h"


/* FTP上传回调函数 */
static size_t upload_read_callback(void *ptr, size_t size, size_t nmemb, void *userp) 
{
    FILE *fp = (FILE *)userp;
    size_t read_size;

    read_size = fread(ptr, size, nmemb, fp);

    return read_size;
}

/* FTP下载回调函数 */
static size_t download_write_callback(void *ptr, size_t size, size_t nmemb, void *userp) 
{
    FILE *fp = (FILE *)userp;
    size_t written;

    written = fwrite(ptr, size, nmemb, fp);

    return written;
}

/* FTP上传文件 */
int ftp_upload(const char *url, const char *local_path, const char *remote_path, const char *user, const char *pass) 
{
    CURL *curl;
    CURLcode res;
    FILE *fp;
    char *ftp_url = NULL;
    struct stat fileInfo;
    curl_off_t fsize;

    spdlog_debug("ftp_upload url:%s, local_path:%s, remote_path:%s, user:%s, pass:%s.", url, local_path, remote_path, user, pass);
    curl = curl_easy_init();
    if (!curl) {
        spdlog_error("Init the curl object failed.");
        return -1;
    }

    if(stat(local_path, &fileInfo)) {
        spdlog_error("Couldnt open '%s': %s.",local_path, strerror(errno));
        return -1;
    }
    fsize = (curl_off_t)fileInfo.st_size;

    fp = fopen(local_path, "rb");
    if (!fp) {
        spdlog_error("Failed to open local file.");
        curl_easy_cleanup(curl);
        return -1;
    }

    if (remote_path != NULL) {
        ftp_url = malloc(strlen(url) + strlen(remote_path) + 2);
        if (!ftp_url) {
            spdlog_error("Memory allocation failed.");
            fclose(fp);
            curl_easy_cleanup(curl);
            return -1;
        }
        sprintf(ftp_url, "%s/%s", url, remote_path);

        curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
        spdlog_debug("ftp_url:%s, fsize %d", ftp_url, fsize);
    } else {
        curl_easy_setopt(curl, CURLOPT_URL, url);
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, upload_read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, fp);
    // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    // curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_FTP);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 20L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 3000L);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, fsize);

    if (user && pass) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pass);
    }

    res = curl_easy_perform(curl);

    if (ftp_url)
        free(ftp_url);
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        spdlog_error("Curl execution failed %s.", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

/* FTP下载文件 */
int ftp_download(const char *url, const char *local_path, const char *remote_path, const char *user, const char *pass) 
{
    CURL *curl;
    CURLcode res;
    FILE *fp;
    char *ftp_url = NULL;

    spdlog_debug("ftp_download url:%s, local_path:%s, remote_path:%s, user:%s, pass:%s.", url, local_path, remote_path, user, pass);
    curl = curl_easy_init();
    if (!curl) {
        spdlog_error("Init the curl object failed.");
        return -1;
    }

    fp = fopen(local_path, "wb");
    if (!fp) {
        spdlog_error("Failed to open local file.");
        curl_easy_cleanup(curl);
        return -1;
    }

    if (remote_path != NULL) {
        ftp_url = malloc(strlen(url) + strlen(remote_path) + 2);
        if (!ftp_url) {
            spdlog_error("Memory allocation failed.");
            fclose(fp);
            curl_easy_cleanup(curl);
            return -1;
        }
        sprintf(ftp_url, "%s/%s", url, remote_path);

        curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
    } else {
        curl_easy_setopt(curl, CURLOPT_URL, url);
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    // curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_FTP);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 20L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 3000L);
    
    if (user && pass) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pass);
    }

    res = curl_easy_perform(curl);

    if (ftp_url)
        free(ftp_url);
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        spdlog_error("Curl execution failed: %s.", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}


/*  回调函数，用于处理服务器返回的数据 */
static size_t post_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    char **response_ptr = (char **)userp;
    *response_ptr = (char *)realloc(*response_ptr, strlen((char *)contents) + 1);
    if (*response_ptr) {
        strcpy(*response_ptr, (char *)contents);
    }
    return real_size;
}

/* 发送 POST 请求的函数 */
int http_post_request(const char *url, const char *json_data, char **response) 
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    curl = curl_easy_init();
    if (!curl) {
        spdlog_error("Failed to initialize CURL.");
        return -1;
    }
    headers = curl_slist_append(headers, "accept: */*");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, post_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        spdlog_error("CURL request failed: %s.", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

