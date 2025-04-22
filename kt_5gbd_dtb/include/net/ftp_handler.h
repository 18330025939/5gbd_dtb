#ifndef __FTP_HANDLER_H
#define __FTP_HANDLER_H

int ftp_upload(const char *url, const char *local_path, const char *remote_path, const char *user, const char *pass);
int ftp_download(const char *url, const char *local_path, const char *remote_path, const char *user, const char *pass);
int http_post_request(const char *url, const char *json_data, char **response);


#endif