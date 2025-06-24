#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <pthread.h>
#include "publib.h"
#include "cJSON.h"
#include "ftp_handler.h"
#include "ssh_client.h"
#include "firmware_updater.h"
#include "spdlog_c.h"


int create_file_upload_data(struct FileUploadfInfo *info, char *data)
{
    cJSON *root = NULL;
    char *buf = NULL;

    if (info == NULL) {
         return -1;
    }

    // spdlog_debug("create_file_upload_data");
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    cJSON_AddStringToObject(root, "deviceAddress", info->dev_addr);
    cJSON_AddStringToObject(root, "filePath", info->upload_path);
    cJSON_AddStringToObject(root, "base64Str", info->base64_str);
    buf = cJSON_Print(root);
    strncpy(data, buf, strlen(buf));
    cJSON_Delete(root);

    free(buf);

    return 0;
}

int do_file_upload(struct FileUploadfInfo *info, char *post_resp)
{
    char buf[1024*500] = {0};
    char *resp = NULL;
    
    int ret = create_file_upload_data(info, buf);
    if (ret) {
        return -1;
    }

    http_post_request(OTA_FILEUPLOAD_URL, buf, &resp);
    spdlog_debug("do_file_upload %s", resp);
    strncpy(post_resp, resp, strlen(resp));
    
    if (resp != NULL) {
        free(resp);
    }
    // if (buf != NULL)
    //     free(buf);

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
    if (strstr(pInfo->type, OTA_TASK_REBOOTED) != NULL) {
        memset(pInfo->type, 0, sizeof(pInfo->type));
        strcpy(pInfo->type, OTA_TASK_NOW);
        return 0;
    }

    UPDATE_LOG_FMT(pInfo->log_path, "start to check %s md5\n", pInfo->name);
    snprintf(cmd, sizeof(cmd), "md5sum %s", pInfo->path);
    _system_(cmd, resp, sizeof(resp));
    if (strstr(resp, pInfo->md5) != NULL) {
        UPDATE_LOG_FMT(pInfo->log_path, "end to check %s md5\n", pInfo->name);
    } else {
        UPDATE_LOG_FMT(pInfo->log_path, "md5 not match, do not excute task %d, ota failed\n", pInfo->id);
        memset(pInfo->type, 0, sizeof(pInfo->type));
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "tar -xzf %s%d/%s -C %s%d/", UPGRADE_FILE_LOCAL_PATH, pInfo->id, pInfo->name, UPGRADE_FILE_LOCAL_PATH, pInfo->id);
    _system_(cmd, NULL, 0);
    snprintf(cmd, sizeof(cmd), "%s%d/rt-a100", UPGRADE_FILE_LOCAL_PATH, pInfo->id);
    if (dir_exists(cmd)) {
        //return 0;
    } else {
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
            //上传失败,是否再次上传?
            SSHClient_Destroy(&ssh_client);
            spdlog_error("ssh_client.upload_file up_file:%s failed.", pInfo->path);
            return -1;
        }

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "md5sum %s", remote_path);
        memset(resp, 0, sizeof(resp));
        ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
        if ((ret) || (strstr(resp, pInfo->md5) == NULL)) {
            //校验失败,是否再次上传?
            SSHClient_Destroy(&ssh_client);
            spdlog_error("ssh_client.execute md5 of the file,l_md5 : %s,r_md5 %s , failed.", pInfo->md5, resp);
            return -1;
        }

        // if (strstr(pInfo->type, OTA_TASK_NEXT) != NULL) {
        //     CustomTime t;
        //     memset(cmd, 0, sizeof(cmd));
        //     snprintf(cmd, sizeof(cmd), "uptime -s");
        //     char uptime[30] = {0};
        //     ret = ssh_client.execute(&ssh_client, cmd, uptime, sizeof(uptime));
        //     if (ret) {
        //         //校验失败,是否再次上传?
        //         SSHClient_Destroy(&ssh_client);
        //         spdlog_error("ssh_client.execute uptime -s failed");
        //         return -1;
        //     }
        // }
        SSHClient_Destroy(&ssh_client);
    }
    

    if (strstr(pInfo->type, OTA_TASK_NOW) != NULL) {
        UPDATE_LOG(pInfo->log_path, "start the ota task now\n");
    } else if (strstr(pInfo->type, OTA_TASK_NEXT) != NULL) {
        UPDATE_LOG(pInfo->log_path, "start the ota task when next reboot\n");
    }

    return 0;
}

int fkz9_fw_update_func(void *arg)
{
    SSHClient ssh_client;
    struct FwUpdateInfo *pInfo = NULL;
    char path[128] = {0};
    char cmd[256] = {0};
    char resp[512] = {0};
    int file_num = 0;
    int i = 0;
    char file_path[80] = {0};
    char buf[1024] = {0};
    size_t bytes = 0;
    
    if (arg  == NULL) {
        return -1;
    }

    spdlog_info("fkz9_fw_update_func");
    pInfo = (struct FwUpdateInfo *)arg;
    if (strstr(pInfo->type, OTA_TASK_NOW) != NULL) {
        snprintf(path, sizeof(path), "%s%d/rt-a100", UPGRADE_FILE_LOCAL_PATH, pInfo->id);
        if (dir_exists(path)) {
            snprintf(cmd, sizeof(cmd), "bash %s%d/run.sh", UPGRADE_FILE_LOCAL_PATH, pInfo->id);
            _system_(cmd, resp, sizeof(resp));
            UPDATE_LOG_FMT(pInfo->log_path, "%s\n", resp);
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

            sscanf(resp, "%[^,],%[^,],%[^,]", pInfo->resp_info.log_path, pInfo->resp_info.conf_path, pInfo->resp_info.down_path);
            spdlog_info("resp: %s, log_path: %s, conf_path: %s", resp, pInfo->resp_info.log_path, pInfo->resp_info.conf_path);

            char local_path[80] = {0};
            snprintf(local_path, sizeof(local_path), "%s%d/%s", UPGRADE_FILE_LOCAL_PATH, pInfo->id, basename(pInfo->resp_info.log_path));
            ret = ssh_client.download_file(&ssh_client, pInfo->resp_info.log_path, local_path);
            if (ret) {
                SSHClient_Destroy(&ssh_client);
                spdlog_error("ssh_client.upload_file down_file:%s failed.", pInfo->resp_info.log_path);
                return -1;
            }

            FILE *fp = fopen(local_path, "r");
            if (fp != NULL) {
                while ((bytes = fread(buf, 1, sizeof(buf) - 1, fp)) > 0) {
                    UPDATE_LOG_FMT(pInfo->log_path, "%s", buf);
                }
                fclose(fp);
            }

            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "wc -l %s", pInfo->resp_info.conf_path);
            memset(resp, 0, sizeof(resp));
            ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
            if (ret) {
                //执行失败
                SSHClient_Destroy(&ssh_client);
                spdlog_error("ssh_client.execute wc -l failed.");
                return -1;
            }

            sscanf(resp, "%d", &file_num);
            if (file_num) {
                // spdlog_info("file_num=%d", file_num);
                struct FileUploadfInfo upload_info;
                for (i = 0; i < file_num; i++) {
                    memset(&upload_info, 0, sizeof(struct FileUploadfInfo));
                    memset(cmd, 0, sizeof(cmd));
                    snprintf(cmd, sizeof(cmd), "sed -n '%dp' %s", i + 1, pInfo->resp_info.conf_path);
                    memset(resp, 0, sizeof(resp));
                    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
                    if (ret) {
                        //执行失败
                        SSHClient_Destroy(&ssh_client);
                        spdlog_error("ssh_client.execute sed -n failed.");
                        return -1;
                    }
                    memset(file_path, 0, sizeof(file_path));
                    sscanf(resp, "%[^,],%[^,],%[^,],", upload_info.dev_addr, file_path, upload_info.upload_path);
                    // spdlog_info("resp : %s, dev_addr : %s, file_path: %s, upload_path : %s", resp, upload_info.dev_addr, file_path, upload_info.upload_path);
                    memset(local_path, 0, sizeof(local_path));
                    snprintf(local_path, sizeof(local_path), "%s%d/%s", UPGRADE_FILE_LOCAL_PATH, pInfo->id, basename(file_path));
                    ret = ssh_client.download_file(&ssh_client, file_path, local_path);
                    if (ret) {
                        SSHClient_Destroy(&ssh_client);
                        spdlog_error("ssh_client.upload_file down_file:%s failed.", pInfo->resp_info.log_path);
                        return -1;
                    }

                    // snprintf(up_info.base64_str, sizeof(up_info.base64_str), "{\"lang\": \"zh_CN\",\"deviceAddress\": \"'\"%s\"'\",\"filePath\": \"'\"%s\"'\",\"base64Str\": 
                    //             \"$(base64 -w 0 %s)\"}", upload_info.dev_addr, upload_info.upload_path, file_path);

                    // memset(cmd, 0, sizeof(cmd));
                    // snprintf(upload_info.base64_str, sizeof(upload_info.base64_str), "curl -X POST -H \"Content-Type: application/json\" -d \"{\"lang\": \"zh_CN\",\"deviceAddress\": \"'\"%s\"'\",\"filePath\": \"'\"%s\"'\",\"base64Str\": 
                    //             \"$(base64 -w 0 %s)\"}\" https://ota.cktt.com.cn/ota-server/fileUpload", upload_info.dev_addr, upload_info.upload_path, file_path);
                    // _system_(upload_info.base64_str, resp, sizeof(resp));
                    // UPDATE_LOG_FMT(pInfo->log_path, "%s", resp);
                    // if (strstr(resp, "SUCCESS") != NULL) {
                    //     UPDATE_LOG_FMT(pInfo->log_path, "upload file %s to cloud success", file_path);
                    // } else {
                    //     UPDATE_LOG_FMT(pInfo->log_path, "upload file %s to cloud failed", file_path);
                    // }


                    memset(cmd, 0, sizeof(cmd));
                    snprintf(cmd, sizeof(cmd), "base64 -w 0 %s", local_path);
                    _system_(cmd, upload_info.base64_str, sizeof(upload_info.base64_str));
                    ret = do_file_upload(&upload_info, resp); 
                    UPDATE_LOG_FMT(pInfo->log_path, "%s\n", resp);
                    if (ret == 0 && strstr(resp, "SUCCESS") != NULL) {
                        UPDATE_LOG_FMT(pInfo->log_path, "upload file %s to cloud success\n", file_path);
                    } else {
                        UPDATE_LOG_FMT(pInfo->log_path, "upload file %s to cloud failed\n", file_path);
                    }
                }

                UPDATE_LOG(pInfo->log_path, "send file end\n");
            }

            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "wc -l %s", pInfo->resp_info.down_path);
            memset(resp, 0, sizeof(resp));
            ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
            if (ret) {
                //执行失败
                SSHClient_Destroy(&ssh_client);
                spdlog_error("ssh_client.execute wc -l failed.");
                return -1;
            }
            sscanf(resp, "%d", &file_num);
            if (file_num) {
                char remote_path[80] = {0};
                for (i = 0; i < file_num; i++) {
                    memset(cmd, 0, sizeof(cmd));
                    snprintf(cmd, sizeof(cmd), "sed -n '%dp' %s", i + 1, pInfo->resp_info.down_path);
                    memset(resp, 0, sizeof(resp));
                    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
                    if (ret) {
                        //执行失败
                        SSHClient_Destroy(&ssh_client);
                        spdlog_error("ssh_client.execute sed -n failed.");
                        return -1;
                    }
                    int dev_addr = 0;
                    sscanf(resp, "%d,%[^,],%[^,]", &dev_addr, file_path, remote_path);
                    memset(cmd, 0, sizeof(cmd));
                    snprintf(cmd, sizeof(cmd), "wget %s -O %s%s 2>&1 | tee -a %swget.log", file_path, UPGRADE_FILE_LOCAL_PATH, basename(remote_path), UPGRADE_FILE_LOCAL_PATH);
                    _system_(cmd, NULL, 0);

                    memset(file_path, 0, sizeof(file_path));
                    snprintf(file_path, sizeof(file_path), "%swget.log", UPGRADE_FILE_LOCAL_PATH);
                    FILE *fp = fopen(file_path, "r");
                    if (fp != NULL) {
                        while ((bytes = fread(buf, 1, sizeof(buf) - 1, fp)) > 0) {
                            UPDATE_LOG_FMT(pInfo->log_path, "%s", buf);
                            memset(buf, 0, sizeof(buf));
                        }
                        fclose(fp);
                    }
                    memset(cmd, 0, sizeof(cmd));
                    snprintf(cmd, sizeof(cmd), "rm -rf %s", file_path);
                    _system_(cmd, NULL, 0);

                    // printf("cmd: %s, resp: %s\n", cmd, resp);
                    memset(file_path, 0, sizeof(file_path));
                    snprintf(file_path, sizeof(file_path), "%s%s", UPGRADE_FILE_LOCAL_PATH, basename(remote_path));
                    ret = ssh_client.upload_file(&ssh_client, file_path, remote_path);
                    if (ret) {
                        //上传失败,是否再次上传?
                        SSHClient_Destroy(&ssh_client);
                        spdlog_error("ssh_client.upload_file up_file:%s failed.", file_path);
                        return -1;
                    }
                    // printf("file_path: %s\n", file_path);
                    memset(cmd, 0, sizeof(cmd));
                    snprintf(cmd, sizeof(cmd), "rm -rf %s", file_path);
                    _system_(cmd, NULL, 0);

                    memset(cmd, 0, sizeof(cmd));
                    snprintf(cmd, sizeof(cmd), "chmod 644 %s & chattr +i %s", remote_path, remote_path);
                    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
                    if (ret) {
                        //执行失败
                        SSHClient_Destroy(&ssh_client);
                        spdlog_error("ssh_client.execute chmod 644 & chattr +i failed.");
                        return -1;
                    }
                }

                memset(cmd, 0, sizeof(cmd));
                snprintf(cmd, sizeof(cmd), "supervisoctl -u cktt -p cktt restart algorithm");
                memset(resp, 0, sizeof(resp));
                ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
                if (ret) {
                    //执行失败
                    SSHClient_Destroy(&ssh_client);
                    spdlog_error("ssh_client.execute supervisoctl -u cktt -p cktt restart algorithm failed.");
                    return -1;
                }
                // printf("supervisoctl -u cktt -p cktt restart algorithm: %s\n", resp);
                // UPDATE_LOG_FMT(pInfo->log_path, "%s\n", resp);
            }
            SSHClient_Destroy(&ssh_client);
        }
    }

    CustomTime t;
    get_system_time(&t);
    snprintf(pInfo->resp_info.time, sizeof(pInfo->resp_info.time), "%04d-%02d-%02d %02d:%02d:%02d", t.usYear, t.ucMonth, t.ucDay, t.ucHour, t.ucMinute, t.ucSecond);
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "base64 %s | tr -d '\n\r'", pInfo->log_path);
    _system_(cmd, pInfo->resp_info.report, sizeof(pInfo->resp_info.report));
    // printf("pInfo->resp_info.report:%s\n", pInfo->resp_info.report);
#if 0
    if (strstr(pInfo->type, OTA_TASK_NOW) != NULL) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s%d", UPGRADE_FILE_LOCAL_PATH, pInfo->id);
        _system_(cmd, NULL, 0);
    }
#endif
    return 0;
}



// int fkz9_fw_update_cb(void *arg)
// {
//     SSHClient ssh_client;
//     struct FwUpdateInfo *pInfo = NULL;
    
//     if (arg == NULL) {
//         return -1;
//     }

//     char cmd[128] = {0};

//     pInfo = (struct FwUpdateInfo *)arg;
    // if (pInfo->resp_info.conf_num) {
        
    //     for (int i = 0; i < pInfo->resp_info.conf_num; i++) {
    //         ftp_upload(, pInfo->resp_info.conf_info[i].path, )
    //     }

    //     free(pInfo->resp_info.conf_info);
    //     UPDATE_LOG
    // }
    // SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    // int ret = ssh_client.connect(&ssh_client);
    // if (ret) {
    //     SSHClient_Destroy(&ssh_client);
    //     spdlog_error("ssh_client.connect failed.");
    //     return -1;
    // }

    // char file_path[256] = {0};
    // snprintf(file_path, sizeof(file_path), "%s/%s", pInfo->path, pInfo->name);
    // ret = ssh_client.download_file(&ssh_client, file_path, pInfo->path);

    // SSHClient_Destroy(&ssh_client);

//     snprintf(cmd, sizeof(cmd), "rm -rf %s", UPGRADE_FILE_LOCAL_PATH);
//     _system_(cmd, NULL, 0);
//     return 0;
// }

REGISTER_FIRMWARE_UPDATE_INTERFACE(fkz9, fkz9_fw_trans_func, fkz9_fw_update_func, NULL);


