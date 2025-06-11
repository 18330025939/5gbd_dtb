#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <pthread.h>
#include "publib.h"
#include "ssh_client.h"
#include "firmware_updater.h"
#include "spdlog_c.h"


int dir_exists(const char *path) 
{
    struct stat statbuf;

    if (stat(path, &statbuf) == 0) { 
        return S_ISDIR(statbuf.st_mode);
    }
    return 0; 
}

int fkz9_fw_trans_func(void *arg)
{
    SSHClient ssh_client;
    struct FwUpdateInfo *pInfo = NULL;
    char cmd[256] = {0};
    char resp[128] = {0};
    
    if (arg  == NULL) {
        return -1;
    }

    pInfo = (struct FwUpdateInfo *)arg;
    UPDATE_LOG_FMT(pInfo->log_path, "start to check %s md5\n", pInfo->name);
    snprintf(cmd, sizeof(cmd), "md5sum %s", pInfo->path);
    _system_(cmd, resp, sizeof(resp));
    if (strstr(resp, pInfo->md5) != NULL) {
        UPDATE_LOG_FMT(pInfo->log_path, "send to check %s md5\n", pInfo->name);
    } 

    snprintf(cmd, sizeof(cmd), "tar -xzvf %s%d/%s -C %s%d/", UPGRADE_FILE_LOCAL_PATH, pInfo->id, pInfo->name, UPGRADE_FILE_LOCAL_PATH, pInfo->id);
    _system_(cmd, NULL, 0);
    snprintf(cmd, sizeof(cmd), "%s%d/%s/rt-a100", UPGRADE_FILE_LOCAL_PATH, pInfo->id, pInfo->md5);
    if (dir_exists(cmd)) {
        return 0;
    }

    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.connect failed.");
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    memset(resp, 0, sizeof(resp));
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/%d", UPGRADE_FILE_REMOTE_PATH, pInfo->id);
    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
    if (ret) {
        //远程路径创建失败
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute %s", cmd);
        return -1;
    }

    char remote_path[128] = {0};
    snprintf(remote_path, sizeof(remote_path), "%s/%d/%s", UPGRADE_FILE_REMOTE_PATH, pInfo->id, pInfo->name);
    ret = ssh_client.upload_file(&ssh_client, pInfo->path, remote_path);
    if (ret) {
        //上传失败是否再次上传
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.upload_file up_file:%s failed.", pInfo->path);
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "md5sum %s", remote_path);
    memset(resp, 0, sizeof(resp));
    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
    if ((ret) || (strstr(resp, pInfo->md5) == NULL)) {
        //校验失败是否再次上传
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute md5 of the file,l_md5 : %s,r_md5 %s , failed.", pInfo->md5, resp);
        return -1;
    }

    SSHClient_Destroy(&ssh_client);

    return 0;
}

int fkz9_fw_update_func(void *arg)
{
    SSHClient ssh_client;
    struct FwUpdateInfo *pInfo = NULL;
    char path[128] = {0};
    char cmd[128] = {0};
    char resp[256] = {0};
    
    if (arg  == NULL) {
        return -1;
    }

    spdlog_info("fkz9_fw_update_func");
    pInfo = (struct FwUpdateInfo *)arg;
    snprintf(path, sizeof(path), "%s%d/%s/rt-a100", UPGRADE_FILE_LOCAL_PATH, pInfo->id, pInfo->md5);
    if (dir_exists(path)) {
        UPDATE_LOG(pInfo->log_path, "start the ota task now\n");
        snprintf(cmd, sizeof(cmd), "bash %s%d/%s/run.sh", UPGRADE_FILE_LOCAL_PATH, pInfo->id, pInfo->md5);
        _system_(cmd, resp, sizeof(resp));
        
        snprintf(cmd, sizeof(cmd), "rm -rf %s", UPGRADE_FILE_LOCAL_PATH);
        _system_(cmd, NULL, 0);
    } else {

        SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
        int ret = ssh_client.connect(&ssh_client);
        if (ret) {
            SSHClient_Destroy(&ssh_client);
            spdlog_error("ssh_client.connect failed.");
            return -1;
        }

        memset(cmd, 0, sizeof(cmd));
        // snprintf(cmd, sizeof(cmd), "bash /home/cktt/script/updater.sh report_info %d %s %s", pInfo->id, pInfo->name, pInfo->type);
        snprintf(cmd, sizeof(cmd), "bash /home/cktt/script/upgrade.sh %d", pInfo->id);
        memset(resp, 0, sizeof(resp));
        ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
        if (ret) {
            //执行失败
            SSHClient_Destroy(&ssh_client);
            spdlog_error("ssh_client.execute updater.sh report_info failed.");
            return -1;
        }

        sscanf(resp, "%[^,],", pInfo->resp_info.log_path);

        char local_path[128] = {0};
        snprintf(local_path, sizeof(local_path), "%s%d/%s/last.log", UPGRADE_FILE_LOCAL_PATH, pInfo->id, pInfo->md5);
        ret = ssh_client.download_file(&ssh_client, pInfo->resp_info.log_path, local_path);
        if (ret) {
            SSHClient_Destroy(&ssh_client);
            spdlog_error("ssh_client.upload_file down_file:%s failed.", pInfo->resp_info.log_path);
            return -1;
        }

        SSHClient_Destroy(&ssh_client);

        FILE *fp = fopen(local_path, "r");
        if (fp != NULL) {
            char buf[1024] = {0};
            size_t bytes = 0;
            while ((bytes = fread(buf, 1, sizeof(buf) - 1, fp)) > 0) {
                UPDATE_LOG_FMT(pInfo->log_path, "%s", buf);
            }
            fclose(fp);
        }
    }

    CustomTime t;
    get_system_time(&t);
    snprintf(pInfo->resp_info.time, sizeof(pInfo->resp_info.time), "%04d-%02d-%02d %02d:%02d:%02d", t.usYear, t.ucMonth, t.ucDay, t.ucHour, t.ucMinute, t.ucSecond);
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "base64 %s | tr -d '\n\r'", pInfo->log_path);
    _system_(cmd, pInfo->resp_info.report, sizeof(pInfo->resp_info.report));
    spdlog_info("remote_log_path:%s, report %s", pInfo->resp_info.log_path, pInfo->resp_info.report);

    // char report_file[64];
    // sscanf(resp, "%s,", report_file);
    // snprintf(report_file, sizeof(report_file), "*_%d_*.log", pInfo->id);
    // snprintf(cmd, sizeof(cmd), "find %s%d -name \"%s\"", OTA_UPREPORT_REMOTE_PATH, pInfo->id, report_file);
    // ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
    // if (ret) {
    //     SSHClient_Destroy(&ssh_client);
    //     spdlog_error("ssh_client.execute ota_report find failed.");
    //     return -1;
    // }
    // char local_path[128] = {0};
    // snprintf(local_path, sizeof(local_path), "%s%d/%s", OTA_UPREPORT_LOCAL_PATH, pInfo->id, basename(resp));
    // ret = ssh_client.download_file(&ssh_client, report_file, local_path);
    // if (ret) {
    //     //下载失败
    //     SSHClient_Destroy(&ssh_client);
    //     spdlog_error("ssh_client.download_file ota_report failed.");
    //     return -1;
    // }

    // SSHClient_Destroy(&ssh_client);
    return 0;
}

int fkz9_fw_update_cb(void *arg)
{
    SSHClient ssh_client;
    struct FwUpdateInfo *pInfo = NULL;
    
    if (arg == NULL) {
        return -1;
    }

    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "rm -rf %s", UPGRADE_FILE_LOCAL_PATH);
    _system_(cmd, NULL, 0);

    pInfo = (struct FwUpdateInfo *)arg;
    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.connect failed.");
        return -1;
    }

    char file_path[256] = {0};
    snprintf(file_path, sizeof(file_path), "%s/%s", pInfo->path, pInfo->name);
    ret = ssh_client.download_file(&ssh_client, file_path, pInfo->path);

    SSHClient_Destroy(&ssh_client);
    return ret;
}

REGISTER_FIRMWARE_UPDATE_INTERFACE(fkz9, fkz9_fw_trans_func, fkz9_fw_update_func, NULL);


