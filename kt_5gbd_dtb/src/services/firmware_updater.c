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
    struct UpdateTask *up_task = (struct UpdateTask *)arg;
    
    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.connect failed.\n");
        return -1;
    }

    char remote_path[128] = {'\0'};
    snprintf(remote_path, sizeof(remote_path), "%s/%2d/", UPGRADE_FILE_REMOTE_PATH, up_task->id);
    ret = ssh_client.upload_file(&ssh_client, up_task->path, remote_path);
    if (ret) {
        //上传失败是否再次上传
    }

    char cmd[256] = {'\0'};
    snprintf(cmd, sizeof(cmd), "md5sum %s/%s", remote_path, up_task->name);
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
    struct UpdateTask *up_task = (struct UpdateTask *)arg;
    
    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.connect failed.\n");
        return -1;
    }

    char cmd[256] = {'\0'};
    snprintf(cmd, sizeof(cmd), "updater.sh report_info");
    char resp[256] = {'\0'};
    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
    if (ret) {
        //执行失败
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute updater.sh report_info failed.\n");
        return -1;
    }

    char report_path[128];
    ret = ssh_client.download_file(&ssh_client, up_task->path, report_path);
    if (ret) {
        //下载失败
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.download_file ota_report failed.\n");
        return -1;
    }

    SSHClient_Destroy(&ssh_client);
    return ret;
}

int fkz9_fw_update_cb(void * arg)
{
    SSHClient ssh_client;
    struct UpdateTask *up_task = (struct UpdateTask *)arg;

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