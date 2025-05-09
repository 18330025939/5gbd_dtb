#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include "ssh_client.h"


static int SSHClient_Connect(SSHClient *client)
{
    ssh_options_set(client->session, SSH_OPTIONS_HOST, client->host);
    ssh_options_set(client->session, SSH_OPTIONS_USER, client->username);

    if (ssh_connect(client->session) != SSH_OK) {
        fprintf(stderr, "Failed to connect to %s: %s\n", client->host, ssh_get_error(client->session));
        return -1;
    }

    if (ssh_userauth_password(client->session, NULL, client->password) != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(client->session));
        return -1;
    }

    client->sftp = sftp_new(client->session);
    if (client->sftp == NULL) {
        fprintf(stderr, "Failed to create SFTP session: %s\n", ssh_get_error(client->session));
        return -1;
    }

    if (sftp_init(client->sftp) != SSH_OK) {
        fprintf(stderr, "Failed to initialize SFTP session: %s\n", ssh_get_error(client->session));
        sftp_free(client->sftp);
        return -1;
    }

    return 0;
}

static int SSHClient_Disconnect(SSHClient *client)
{
    if (client->sftp) {
        // sftp_shutdown(client->sftp);
        sftp_free(client->sftp);
        client->sftp = NULL;
    }
    if (client->session) {
        ssh_disconnect(client->session);
        ssh_free(client->session);
        client->session = NULL;
    }
    return 0;
}

static int SSHClient_Execute(SSHClient *client, const char *command, char *output, size_t output_size)
{
    ssh_channel channel = ssh_channel_new(client->session);
    if (channel == NULL) {
        fprintf(stderr, "Failed to create channel: %s\n", ssh_get_error(client->session));
        return -1;
    }

    if (ssh_channel_open_session(channel) != SSH_OK) {
        fprintf(stderr, "Failed to open channel: %s\n", ssh_get_error(client->session));
        ssh_channel_free(channel);
        return -1;
    }

    if (ssh_channel_request_exec(channel, command) != SSH_OK) {
        fprintf(stderr, "Failed to execute command: %s\n", ssh_get_error(client->session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return -1;
    }

    char buffer[1024];
    int nbytes = 0;
    int recv_bytes = 0;
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
        strncat(output, buffer, nbytes);
        recv_bytes += nbytes;
    }
    output[recv_bytes] = '\0';

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return 0;
}

static int SSHClient_UploadFile(SSHClient *client, const char *local_path, const char *remote_path)
{
    if(client->sftp == NULL) {
        return -1;
    }

    FILE *local_file = fopen(local_path, "rb");
    if (local_file == NULL) {
        fprintf(stderr, "Failed to open local file: %s\n", local_path);
        return -1;
    }

    sftp_file remote_file = sftp_open(client->sftp, remote_path, O_WRONLY | O_CREAT , 0644);
    if (remote_file == NULL) {
        fprintf(stderr, "Failed to open remote file: %s\n", ssh_get_error(client->session));
        fclose(local_file);
        return -1;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), local_file)) > 0) {
        if (sftp_write(remote_file, buffer, bytes_read) < 0) {
            fprintf(stderr, "Failed to write to remote file: %s\n", ssh_get_error(client->session));
            fclose(local_file);
            sftp_close(remote_file);
            return -1;
        }
    }

    fclose(local_file);
    sftp_close(remote_file);
    return 0;
}

static int SSHClient_DownloadFile(SSHClient *client, const char *remote_path, const char *local_path)
{
    if (client->sftp == NULL) {
        return -1;
    }

    sftp_file remote_file = sftp_open(client->sftp, remote_path, O_RDONLY, 0);
    if (remote_file == NULL) {
        fprintf(stderr, "Failed to open remote file: %s\n", ssh_get_error(client->session));
        return -1;
    }

    FILE *local_file = fopen(local_path, "wb");
    if (local_file == NULL) {
        fprintf(stderr, "Failed to open local file: %s\n", local_path);
        sftp_close(remote_file);
        return -1;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = sftp_read(remote_file, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_read, local_file);
    }

    fclose(local_file);
    sftp_close(remote_file);
    return 0;
}

void SSHClient_Init(SSHClient *client, const char *host, const char *username, const char *password)
{
    client->host = strdup(host);
    client->username = strdup(username);
    client->password = strdup(password);
    client->session = ssh_new();
    client->sftp = NULL;

    // 初始化函数指针
    client->connect = SSHClient_Connect;
    client->disconnect = SSHClient_Disconnect;
    client->execute = SSHClient_Execute;
    client->upload_file = SSHClient_UploadFile;
    client->download_file = SSHClient_DownloadFile;
}

void SSHClient_Destroy(SSHClient *client)
{
    free(client->host);
    free(client->username);
    free(client->password);
    if (client->sftp) {
        // sftp_shutdown(client->sftp);
        sftp_free(client->sftp);
    }
    if (client->session) {
        ssh_disconnect(client->session);
        ssh_free(client->session);
    }
}

