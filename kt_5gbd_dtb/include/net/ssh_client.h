#ifndef __SSH_CLIENT_H
#define __SSH_CLIENT_H


typedef struct {
    ssh_session session; // SSH 会话
    sftp_session sftp;   // SFTP 会话
    char *host;          // 主机地址
    char *username;      // 用户名
    char *password;      // 密码

    // 操作函数指针
    int (*connect)(struct SSHClient *self);
    int (*disconnect)(struct SSHClient *self);
    int (*execute)(struct SSHClient *self, const char *command, char *output, size_t output_size);
    int (*upload_file)(struct SSHClient *self, const char *local_path, const char *remote_path);
    int (*download_file)(struct SSHClient *self, const char *remote_path, const char *local_path);
} SSHClient;

void SSHClient_Init(SSHClient *client, const char *host, const char *username, const char *password);
void SSHClient_Destroy(SSHClient *client);

#endif
