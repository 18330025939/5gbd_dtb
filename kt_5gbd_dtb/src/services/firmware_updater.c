#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "ssh_client.h"
#include "firmware_updater.h"

int fkz9_fw_trans_func(void *arg)
{
    SSHClient ssh_client;
    UpdateTask *up_task = (UpdateTask *)arg;
    
    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.connect failed.\n");
        return -1;
    }

    char file_path[256] = {'\0'};
    snprintf(file_path, sizeof(file_path), "%s/%s", up_task->path, up_task->name);
    ret = ssh_client.upload_file(&ssh_client, file_path, up_task->path);
    if (ret) {
        //上传失败是否再次上传
    }

    char cmd[256] = {'\0'};
    snprintf(cmd, sizeof(cmd), "md5sum %s/%s", up_task->path, up_task->name);
    char resp[256] = {'\0'};
    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
    if (strcmp(up_task->md5, resp) != 0) {
        //校验失败是否再次上传
    }

    SSHClient_Destroy(&ssh_client);
    return ret;
}

int fkz9_fw_update_func(void *arg)
{
    SSHClient ssh_client;
    UpdateTask *up_task = (UpdateTask *)arg;
    
    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.connect failed.\n");
        return -1;
    }

    char cmd[256] = {'\0'};
    snprintf(cmd, sizeof(cmd), "md5sum %s/%s", up_task->path, up_task->name);
    char resp[256] = {'\0'};
    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
    if (ret) {
        //上传失败是否再次上传
    }

    SSHClient_Destroy(&ssh_client);
    return ret;
}

int fkz9_fw_update_cb(void * arg)
{
    SSHClient ssh_client;
    UpdateTask *up_task = (UpdateTask *)arg;

    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.connect failed.\n");
        return -1;
    }

    char file_path[256] = {'\0'};
    snprintf(file_path, sizeof(file_path), "%s/%s", up_task->path, up_task->name);
    ret = ssh_client.download_file(&ssh_client, file_path, up_task->path);

    SSHClient_Destroy(&ssh_client);
    return ret;
}

REGISTER_FIRMWARE_UPDATE_INTERFACE(fkz9, fkz9_fw_trans_func, fkz9_fw_update_func, fkz9_fw_update_cb);