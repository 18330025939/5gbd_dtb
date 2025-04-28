#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <dirent.h>
#include "serial.h"
#include "fx650.h"

#define SYS_NET_PATH "/sys/class/net"
#define MAX_LINE_LEN 256
#define PRODUCT_PREFIX "PRODUCT="

// 检查uevent文件中是否存在匹配的PRODUCT字段
static int check_uevent_product(const char *uevent_path, const char *target_vid, const char *target_pid) 
{
    FILE *fp = fopen(uevent_path, "r");
    if (!fp) {
        perror("fopen uevent failed");
        return -1;
    }

    char line[MAX_LINE_LEN];
    int found = 0;
    char expected_product[32];
    snprintf(expected_product, sizeof(expected_product), "%s/%s", target_vid, target_pid);

    while (fgets(line, sizeof(line), fp)) {
        // 查找PRODUCT=开头的行
        if (strncmp(line, PRODUCT_PREFIX, strlen(PRODUCT_PREFIX)) == 0) {
            // 提取PRODUCT值（格式：vid/pid/...）
            char *product = line + strlen(PRODUCT_PREFIX);
            // printf("product %s\n", product);
            char *first_slash = strchr(product, '/');
            if (!first_slash) continue;

            // 截取前两部分（vid/pid）
            *first_slash = '\0';
            char *vid = product;
            // printf("vid %s\n", vid);
            char *pid = first_slash + 1;
            // printf("pid %s\n", pid);
            char *second_slash = strchr(pid, '/');
            if (second_slash) *second_slash = '\0';

            // 比较VID/PID
            if (strcasecmp(vid, target_vid) == 0 && 
                strcasecmp(pid, target_pid) == 0) {
                found = 1;
                break;
            }
        }
    }

    fclose(fp);
    return found;
}

// 获取匹配的网卡名称
static char* find_interface_by_vid_pid(const char *vid, const char *pid) 
{
    DIR *net_dir;
    struct dirent *entry;
    char *found_ifname = NULL;

    // 打开网络设备目录
    if ((net_dir = opendir(SYS_NET_PATH)) == NULL) {
        perror("opendir failed");
        return NULL;
    }

    // 遍历所有网络接口
    while ((entry = readdir(net_dir)) != NULL) {
        // 跳过非目录和特殊目录
        if (entry->d_type != DT_LNK || 
            strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构建设备路径
        char device_path[300];
        snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_NET_PATH, entry->d_name);
        // printf("device path: %s\n", device_path);
        // 解析符号链接获取真实路径
        char real_device_path[512];
        if (realpath(device_path, real_device_path) == NULL) {
            continue;
        }
        // printf("real device path: %s\n", real_device_path);
        // 构建uevent文件路径
        char uevent_path[1024];
        snprintf(uevent_path, sizeof(uevent_path), "%s/uevent", real_device_path);
        // printf("uevent path: %s\n", uevent_path);
        // 检查uevent文件
        if (access(uevent_path, R_OK) == 0) {
            if (check_uevent_product(uevent_path, vid, pid)) {
                found_ifname = strdup(entry->d_name);
                break;
            }
        }
    }

    closedir(net_dir);
    return found_ifname;
}

// 发送AT指令核心函数
static FX650_Error send_at_command(Fx650Ctx* ctx, const char* cmd,
                                  char* resp, size_t resp_len,
                                  uint32_t timeout_ms) 
{

    // 发送命令
    ctx->uart->base.ops->write(&ctx->uart->base, cmd, strlen(cmd));
    
    fd_set read_set;
    struct timeval timeout;

    while (1) {
        FD_ZERO(&read_set);
		FD_SET(ctx->uart->base.fd, &read_set);
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;

        if (select(ctx->uart->base.fd + 1, &read_set, NULL, NULL, &timeout) <= 0) {
			return FX650_ERR_AT_TIMEOUT;
		}

        ssize_t n = ctx->uart->base.ops->read(&ctx->uart->base, resp, resp_len);
        // if (n <= 0) continue;
        resp[n] = '\0';
	    // printf("%s --,%ld\n", resp, n);

        // 检查是否收到OK或ERROR
        if (strstr(resp, "OK\r\n")) {
            return FX650_OK;
        }
        if (strstr(resp, "\r\nERROR\r\n") || strstr(resp, "+CME ERROR:")) {
            return FX650_ERR_AT_ERROR;
        }
    }
}

// 检查SIM卡状态
static int check_sim_status(Fx650Ctx* ctx) 
{
    char resp[AT_MAX_RESPONSE_LEN];
    if (send_at_command(ctx, "AT+CPIN?\r\n", resp, sizeof(resp), AT_TIMEOUT_MS) < 0) {
        return -1;
    }

    if (strstr(resp, "+CPIN: READY") == NULL) {
        fprintf(stderr, "SIM card not ready.\n");
        return -1;
    }
    return 0;
}

// 设置APN
/*  cmiot：中国移动的4G和5G网络。
    5gnet：中国联通的4G和5G网络。
    ctlte：中国电信的4G和5G网络。
*/
static int set_apn(Fx650Ctx* ctx, const char *apn) 
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CGDCONT=1,\"IP\",\"%s\"\r", apn);
    char resp[AT_MAX_RESPONSE_LEN];
    if (send_at_command(ctx, cmd, resp, sizeof(resp), AT_TIMEOUT_MS) < 0) {
        return -1;
    }

    if (strstr(resp, "OK") == NULL) {
        fprintf(stderr, "APN setting failed.\n");
        return -1;
    }

    return 0;
}

// 激活拨号
static int activate_dia(Fx650Ctx* ctx, uint8_t status) 
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+GTRNDIS=%s,1\r", status ? "1" : "0");
    char resp[AT_MAX_RESPONSE_LEN];
    if (send_at_command(ctx, cmd, resp, sizeof(resp), AT_TIMEOUT_MS) < 0) {
        return -1;
    }

    // 检查是否返回OK
    if (strstr(resp, "OK") == NULL) {
        fprintf(stderr, "Activation dialing failed.\n");
        return -1;
    }
    if (status) {
        // 查询IP分配状态
        if (send_at_command(ctx, "AT+GTRNDIS?\r", resp, sizeof(resp), AT_TIMEOUT_MS) < 0) {
            return -1;
        }

        if (strstr(resp, "OK") == NULL) {
            fprintf(stderr, "IP address not obtained.\n");
            return -1;
        }
    }

    return 0;
}

static int check_network_connection(char *net, const char *hostname)
{
    char cmd[256];

    sprintf(cmd, "ping -I %s -c 1 %s > /dev/null 2>&1", net, hostname);
    int ret = system(cmd);

    return ret == 0;
}

// 执行DHCP获取IP
static int run_dhcp_client(const char* net) 
{
    char cmd[128];
    int ret = 0;

    snprintf(cmd, sizeof(cmd), "ip link set %s up", net);
    if((ret = system(cmd)) == -1) {
        return ret;
    }

    snprintf(cmd, sizeof(cmd), "udhcpc -i %s &", net);
    if((ret = system(cmd)) == -1) {
        return ret;
    }

    snprintf(cmd, sizeof(cmd), "ip route del default");
    if((ret = system(cmd)) == -1) {
        return ret;
    }

    snprintf(cmd, sizeof(cmd), "ip route add default via 192.168.10.1 dev %s metric 600", net);
    if((ret = system(cmd)) == -1) {
        return ret;
    }

    snprintf(cmd, sizeof(cmd), "ip route add default via 192.168.42.1 dev eth0 metric 20100");
    if((ret = system(cmd)) == -1) {
        return ret;
    }
    do {
        sleep(100);
        if (check_network_connection(net, "www.baidu.com")) {
            break;
        }
    } while (1);

    return 0;
}

FX650_Error fx650_connect_network(Fx650Ctx* ctx) 
{
    int ret = 0;
    ret = check_sim_status(ctx);
    if (ret) {
        return FX650_ERR_SIM_NOT_READY;
    }
    ret = set_apn(ctx, "5gnet");
    if(ret) {
        return FX650_ERR_APN_NOT_READY;
    }
    ret = activate_dia(ctx, 1);
    if (ret) {
        fprintf(stderr, "Activation of RNDIS failed,attempt to deactivate and reactivate.\n");
        if (activate_dia(ctx, 0)) {
            fprintf(stderr, "Failed to deactivate RNDIS.\n");
            return FX650_ERR_PDP_ACTIVATE;
        }
        if (activate_dia(ctx, 1)) {
            fprintf(stderr, "Activation failed again, exit.\n");
            return FX650_ERR_PDP_ACTIVATE;
        }
    }

    run_dhcp_client(ctx->net_name);
    return FX650_OK;
}

FX650_Error fx650_disconnect_network(Fx650Ctx* ctx) 
{
    int ret = 0;

    ret = activate_dia(ctx, 0);
    if (ret) {
        return FX650_ERR_PDP_ACTIVATE;
    }

    return FX650_OK;
}

FX650_Error fx650_init(Fx650Ctx* ctx) 
{
    UartPort *fx650_port = NULL;

    ctx->net_name = find_interface_by_vid_pid(FX650_VID, FX650_PID);
    if (ctx->net_name == NULL) {
        fprintf(stderr, "Network port name not found.\n");
        return FX650_ERR_INIT;
    }
    printf("Network port name: %s\n", ctx->net_name);   
    if (check_network_connection(ctx->net_name, "www.baidu.com")) {
        return FX650_OK;
    }

    SerialPortInfo fx650_port_info = {
        .speed = 115200, 
        .data_bits = 8, 
        .stop_bits = 1, 
        .parity = 'N', 
        .fctl = 0
    };
    fx650_port = uart_port_create();
    int code = fx650_port->base.ops->open(&fx650_port->base, FX650_DEV_NAME);
    if (code) {
        fprintf(stderr, "Failed to open serial port %s.\n", FX650_DEV_NAME);
        return FX650_ERR_INIT;
    }
    fx650_port->base.ops->configure(&fx650_port->base, &fx650_port_info);

    ctx->uart = fx650_port;

    // 发送基础AT指令测试
    char resp[64];
    FX650_Error ret = send_at_command(ctx, "AT\r\n", resp, sizeof(resp), AT_TIMEOUT_MS);
    if (ret != FX650_OK) {
        fprintf(stderr, "Failed to send AT command.\n");
        close(ctx->uart->base.fd);
        return ret;
    }

    // 关闭回显
    // ret = send_at_command(ctx, "ATE0\r", resp, sizeof(resp), AT_TIMEOUT_MS);
    return ret;
}

void fx650_uninit(Fx650Ctx* ctx)
{
    UartPort *fx650_port = NULL;

    if (ctx == NULL) {
        return ;
    }

    if (ctx->net_name) {
        free(ctx->net_name);
    }
    fx650_port = ctx->uart;
    if (fx650_port->base.is_open) {
        fx650_port->base.ops->close(&fx650_port->base);
    }
    if (fx650_port) {
        free(fx650_port);
    }
    free(ctx);
} 


