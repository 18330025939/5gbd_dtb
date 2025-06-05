#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <event2/event.h>
#include "spdlog_c.h"
#include "firmware_updater.h"
#include "cloud_comm.h"
#include "fkz9_comm.h"
#include "led.h"
#include "VersionInfo.h"
#include "ssh_client.h"


#if !defined(SPDLOG_ACTIVE_LEVEL)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

int init_updater_environment(void)
{
    SSHClient ssh_client;

    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.connect failed.");
        return -1;
    }

    char resp[128] = {0};
    ret = ssh_client.execute(&ssh_client, "find /home/cktt/script/ -name \"updater.sh\"", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute find updater.sh failed.");
        return -1;
    } else {
        spdlog_info("find updater.sh resp %s.", resp);
        if (strstr(resp, "updater.sh")) {
            memset(resp, 0, sizeof(resp));
            ret = ssh_client.execute(&ssh_client, "bash /home/cktt/script/updater.sh dev_info", resp, sizeof(resp));
            if (ret) {
                SSHClient_Destroy(&ssh_client);
                spdlog_error("ssh_client.execute updater.sh dev_info failed.");
                return -1;
            }
            memset((void*)&fkz9_devBaseInfo, 0, sizeof(DevbaseInfo));
            sscanf(resp, "%hd,%[^,],%d,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,", 
                    &fkz9_devBaseInfo.dev_addr,
                    fkz9_devBaseInfo.cloud_ip,
                    &fkz9_devBaseInfo.cloud_port,
                    &fkz9_devBaseInfo.cpu_sw,
                    &fkz9_devBaseInfo.cpu_hw,
                    &fkz9_devBaseInfo.ad_sw,
                    &fkz9_devBaseInfo.ad_hw,
                    &fkz9_devBaseInfo.ctrl_sw,
                    &fkz9_devBaseInfo.ctrl_hw,
                    &fkz9_devBaseInfo.net_sw,
                    &fkz9_devBaseInfo.net_hw);
            spdlog_info("dev_addr:%d, cloud_ip:%s, cloud_port:%d,", fkz9_devBaseInfo.dev_addr, fkz9_devBaseInfo.cloud_ip, fkz9_devBaseInfo.cloud_port);

            SSHClient_Destroy(&ssh_client);
            return 0;
        }
    }
    
    char loacl_path[256];
    _system_("pwd", resp, sizeof(resp));
    snprintf(loacl_path, sizeof(loacl_path), "%s/kt_5gbd_dtb/script/updater.sh", resp);
    ret = ssh_client.upload_file(&ssh_client, loacl_path, "/home/cktt/script/updater.sh");
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.upload_file updater.sh failed.");
        return -1;
    }

    ret = ssh_client.execute(&ssh_client, "chmod +x /home/cktt/script/updater.sh", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute chmod +x updater.sh failed.");
        return -1;
    }

    SSHClient_Destroy(&ssh_client);
    return 0;
}

void signal_handler(evutil_socket_t fd, short events, void *arg)
{
    struct event_base *base = NULL;
    int signal = fd;

    base = (struct event_base *)arg;
    spdlog_info("Received signal %s (signal %d),quit...", strsignal(signal), signal);
    event_base_loopexit(base, NULL);
}

static void *event_task_entry(void *arg)
{
    struct event_base *base = event_base_new();
    if (!base) {
        spdlog_error("Could not create event base: exiting");
        return NULL;
        //exit(1);
    }
    struct event *sigint = evsignal_new(base, SIGINT, signal_handler, base);
    if (!sigint) {
        spdlog_error("Could not create SIGINT event: exiting");
        event_base_free(base);
        return NULL;
        //exit(1);
    }
    event_add(sigint, NULL);

    struct event *sigterm = evsignal_new(base, SIGTERM, signal_handler, base);
    if (!sigterm) {
        spdlog_error("Could not create SIGTERM event: exiting");
        event_free(sigint);
        event_base_free(base);
        return NULL;
        //exit(1);
    }
    event_add(sigterm, NULL);

    event_base_dispatch(base);

    event_free(sigint);
    event_free(sigterm);
    event_base_free(base);

    return NULL;
}

int main(int argc, char ** args)
{
    pthread_t event_thread;
    CloundCommContext *cloud_ctx = NULL;
    Fkz9CommContext *fkz9_ctx = NULL;
    
    spdlog_c_init("/opt/log/rt_a100.log", 1024 * 2, 2);
    spdlog_info("BUILD_TIMESTAMP: %s",BUILD_TIMESTAMP);
    spdlog_info("CLIENT_VERSION: %s", CLIENT_VERSION);
    spdlog_info("RT_A100_VERSION_MAJOR: %d.%d.%d", RT_A100_VERSION_MAJOR, RT_A100_VERSION_MINOR, RT_A100_VERSION_PATCH);

    cloud_ctx = (CloundCommContext*)malloc(sizeof(CloundCommContext));
    if (cloud_ctx == NULL) {
        exit(1);
    }
    memset(cloud_ctx, 0, sizeof(CloundCommContext));
    
    fkz9_ctx = (Fkz9CommContext*)malloc(sizeof(Fkz9CommContext));
    if (fkz9_ctx == NULL) {
        free(cloud_ctx);
        exit(1);
    }
    // RUN_LED_INIT();
    // FAULT_LED_INIT();

    init_updater_environment();
    clound_comm_init(cloud_ctx);
    fkz9_comm_init(fkz9_ctx);

    pthread_create(&event_thread, NULL, event_task_entry, NULL);
    
    pthread_join(event_thread, NULL);
    clound_comm_uninit(cloud_ctx); 
    fkz9_comm_uninit(fkz9_ctx);
 //   FAULT_LED_ON();
    free(cloud_ctx);
    free(fkz9_ctx);

    spdlog_info("Application exiting...");

    exit(0);
}
