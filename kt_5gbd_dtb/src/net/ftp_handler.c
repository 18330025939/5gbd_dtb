#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <curl/curl.h>

// FTP上传回调函数
static size_t upload_read_callback(void *ptr, size_t size, size_t nmemb, void *userp) 
{
    FILE *fp = (FILE *)userp;
    size_t read_size;

    read_size = fread(ptr, size, nmemb, fp);

    return read_size;
}

// FTP下载回调函数
static size_t download_write_callback(void *ptr, size_t size, size_t nmemb, void *userp) 
{
    FILE *fp = (FILE *)userp;
    size_t written;

    written = fwrite(ptr, size, nmemb, fp);

    return written;
}

// FTP上传文件
int ftp_upload(const char *url, const char *local_path, const char *remote_path, const char *user, const char *pass) 
{
    CURL *curl;
    CURLcode res;
    FILE *fp;
    struct stat fileInfo;
    curl_off_t fsize;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Init the curl object failed\n");
        return -1;
    }

    if(stat(local_path, &fileInfo)) {
        fprintf(stderr, "Couldnt open '%s': %s\n",local_path, strerror(errno));
        return -1;
    }
    fsize = (curl_off_t)fileInfo.st_size;

    fp = fopen(local_path, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open local file\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    char *ftp_url = malloc(strlen(url) + strlen(remote_path) + 2);
    if (!ftp_url) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        curl_easy_cleanup(curl);
        return -1;
    }
    sprintf(ftp_url, "%s/%s", url, remote_path);

    curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, upload_read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, fp);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_FTP);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 20L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 3000L);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, fsize);

    if (user && pass) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pass);
    }

    res = curl_easy_perform(curl);

    free(ftp_url);
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Curl execution failed %s\n", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

// FTP下载文件
int ftp_download(const char *url, const char *local_path, const char *remote_path, const char *user, const char *pass) 
{
    CURL *curl;
    CURLcode res;
    FILE *fp;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Init the curl object failed\n");
        return -1;
    }

    fp = fopen(local_path, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open local file\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    char *ftp_url = malloc(strlen(url) + strlen(remote_path) + 2);
    if (!ftp_url) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        curl_easy_cleanup(curl);
        return -1;
    }
    sprintf(ftp_url, "%s/%s", url, remote_path);

    curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_FTP);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 20L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 3000L);
    
    if (user && pass) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pass);
    }

    res = curl_easy_perform(curl);

    free(ftp_url);
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Curl execution failed: %s\n", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}


// 回调函数，用于处理服务器返回的数据
static size_t post_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    char **response_ptr = (char **)userp;
    *response_ptr = (char *)realloc(*response_ptr, strlen((char *)contents) + 1);
    if (*response_ptr) {
        strcpy(*response_ptr, (char *)contents);
    }
    return real_size;
}

// 发送 POST 请求的函数
int http_post_request(const char *url, const char *json_data, char **response) 
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return -1;
    }
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
        fprintf(stderr, "CURL request failed: %s\n", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}
