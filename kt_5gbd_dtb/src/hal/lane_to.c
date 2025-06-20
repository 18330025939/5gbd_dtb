#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "serial.h"
#include "lane_to.h"


extern struct MessageParser __start_message_parsers;
extern struct MessageParser __stop_message_parsers;

#if 0
void calculate_checksum(const uint8_t *payload, uint16_t len, uint8_t *ckA, uint8_t *ckB) 
{
    uint16_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += payload[i];
    }
    *ckA = (uint8_t)(sum & 0xFF);
    *ckB = (uint8_t)((sum >> 8) & 0xFF);
}

/* 构造输入命令的通用函数 */
uint8_t* build_command(uint8_t cmd_id, const uint8_t *payload, uint16_t payload_len) 
{
    BinaryCommandHeader *cmd = malloc(sizeof(BinaryCommandHeader) + payload_len + 2);
    cmd->sync1 = 0x50;
    cmd->sync2 = 0x42;
    cmd->msg_id = cmd_id;
    cmd->payload_len = payload_len;
    malloc(cmd->payload, payload_len);
    memcpy(cmd->payload, payload, payload_len);
    
    // 计算校验和
    calculate_checksum(payload, payload_len, &cmd->payload[payload_len], &cmd->payload[payload_len + 1]);
    return (uint8_t*)cmd;
}

uint8_t* build_set_nav_rate_command(NavRate rate, bool save_to_flash) 
{
    uint8_t payload = rate | (save_to_flash ? 0x80 : 0x00); // 最高位表示保存配置
    return build_command(CMD_SET_NAV_RATE, &payload, 1);
}

uint8_t* build_set_baud_command(BaudRate rate, bool save_to_flash) 
{
    uint8_t payload = rate | (save_to_flash ? 0x80 : 0x00);
    return build_command(CMD_BAUD_RATE_SETTING, &payload, 1);
}

/* NMEA校验和验证 */
int validate_nmea_checksum(const char *nmea_str) {
    uint8_t calc_checksum = 0;
    const char *p = nmea_str + 1; // 跳过'$'
    while (*p != '*' && *p != '\0') {
        calc_checksum ^= *p++;
    }
    if (*p != '*') return -1;
    uint8_t recv_checksum = strtoul(p+1, NULL, 16);
    return (calc_checksum == recv_checksum) ? 0 : -2;
}
#endif
static void sg_data_parse(void *data, const char *payload, size_t len)
{
    SGData *sg = (SGData*)data;

    sscanf((const char *)payload, "%[^,],%hhu,%hu,%hhu,%hhu,%hhu,%hhu,%hu,%u,%hu,%u,%u,%lf,%lf,%f,%f,%d,%d,%d,"
            "%u,%u,%d,%d,%d,%d,%d,%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hd,%hd,%hd,%hu,%hhu,%hhu,%hhu",
            sg->message_id,
            &sg->subid,
            &sg->utc_year,
            &sg->utc_month,
            &sg->utc_day,
            &sg->utc_hour,
            &sg->utc_minutes,
            &sg->utc_millisecond,
            &sg->sensor_time,
            &sg->gps_week_number,
            &sg->tow,
            &sg->nav_mode_info,
            &sg->latitude,
            &sg->longitude,
            &sg->altitude_ellipsoid,
            &sg->altitude_msl,
            &sg->vn,
            &sg->ve,
            &sg->vd,
            &sg->ground_speed,
            &sg->traveled_distance,
            &sg->roll,
            &sg->pitch,
            &sg->heading,
            &sg->reserved1,
            &sg->reserved2,
            &sg->north_uncertainty,
            &sg->east_uncertainty,
            &sg->down_uncertainty,
            &sg->vn_uncertainty,
            &sg->ve_uncertainty,
            &sg->vd_uncertainty,
            &sg->roll_uncertainty,
            &sg->pitch_uncertainty,
            &sg->yaw_uncertainty,
            &sg->misalign_angle_roll,
            &sg->misalign_angle_pitch,
            &sg->misalign_angle_yaw,
            &sg->reference_station_id,
            &sg->time_since_last_diff,
            &sg->reserved[0],
            &sg->reserved[1]);
    // printf("sg_data_parse: %s,%hhu,%hu,%hhu,%hhu,%hhu,%hhu,%hu,%u,%hu,%u,%u,%.8lf,%.8lf,%f,%f,%d,%d,%d,"
    //        "%u,%u,%d,%d,%d,%d,%d,%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hd,%hd,%hd,%hu,%hhu,%hhu,%hhu\n",
    //         sg->message_id,
    //         sg->subid,
    //         sg->utc_year,
    //         sg->utc_month,
    //         sg->utc_day,
    //         sg->utc_hour,
    //         sg->utc_minutes,
    //         sg->utc_millisecond,
    //         sg->sensor_time,
    //         sg->gps_week_number,
    //         sg->tow,
    //         sg->nav_mode_info,
    //         sg->latitude,
    //         sg->longitude,
    //         sg->altitude_ellipsoid,
    //         sg->altitude_msl,
    //         sg->vn,
    //         sg->ve,
    //         sg->vd,
    //         sg->ground_speed,
    //         sg->traveled_distance,
    //         sg->roll,
    //         sg->pitch,
    //         sg->heading,
    //         sg->reserved1,
    //         sg->reserved2,
    //         sg->north_uncertainty,
    //         sg->east_uncertainty,
    //         sg->down_uncertainty,
    //         sg->vn_uncertainty,
    //         sg->ve_uncertainty,
    //         sg->vd_uncertainty,
    //         sg->roll_uncertainty,
    //         sg->pitch_uncertainty,
    //         sg->yaw_uncertainty,
    //         sg->misalign_angle_roll,
    //         sg->misalign_angle_pitch,
    //         sg->misalign_angle_yaw,
    //         sg->reference_station_id,
    //         sg->time_since_last_diff,
    //         sg->reserved[0],
    //         sg->reserved[1]);

    return ;
}
REGISTER_MESSAGE_PARSER(PBSOL, 1, &sg_data, sg_data_parse);

#if 0
static void gngga_data_parse(void *data, const char *payload, size_t len)
{   
    GNGGAData *gngga = (GNGGAData*)data;

    // char buffer[512];
    // strncpy(buffer, payload, len);
    // buffer[len] = '\0';

    // char *token = strtok((char *)payload, ",*"); // 分割到校验和前
    // if (!token || strncmp(token, GNGGA_MSG_ID, 5) != 0) {
    //     return ; // 非PBSOL消息
    // }

    sscanf((const char *)payload, "%[^,],%lf,%lf,%c,%lf,%c,%hhu,%hhu,"
            "%f,%f,%c,%f,%c",
            gngga->talker_id,
            &gngga->time,
            &gngga->latitude,
            &gngga->ns,
            &gngga->longitude,
            &gngga->ew,
            &gngga->fix_quality,
            &gngga->num_satellites,
            &gngga->hdop,
            &gngga->altitude_msl,
            &gngga->altitude_unit,
            &gngga->geoid_separation,
            &gngga->geoid_unit);

    // printf("gngga_data_parse: %s,%lf,%lf,%c,%lf,%c,%hhu,%hhu,%f,%f,%c,%f,%c\n",
    //         gngga->talker_id,
    //         gngga->time,
    //         gngga->latitude,
    //         gngga->ns,
    //         gngga->longitude,
    //         gngga->ew,
    //         gngga->fix_quality,
    //         gngga->num_satellites,
    //         gngga->hdop,
    //         gngga->altitude_msl,
    //         gngga->altitude_unit,
    //         gngga->geoid_separation,
    //         gngga->geoid_unit);
    // char *checksum_str = strrchr(payload, '*');
    // if (checksum_str) {
    //     uint8_t calc_checksum = 0;
    //     for (char *p = buffer + 1; p < checksum_str; p++) {
    //         calc_checksum ^= *p;
    //     }
    //     uint8_t recv_checksum = (uint8_t)strtoul(checksum_str + 1, NULL, 16);
    //     if (calc_checksum != recv_checksum) return -3;
    // }
    return ;
}
// REGISTER_MESSAGE_PARSER(GNGGA, 0, &gngga_data, gngga_data_parse);

static void gnrmc_data_parse(void *data, const char *payload, size_t len)
{
    GNRMCData *gnrmc = (GNRMCData*)data;

    sscanf((const char *)payload, "%[^,],%lf,%c,%lf,%c,%lf,%c,%f,%f,%u",
            gnrmc->talker_id,
            &gnrmc->time,
            &gnrmc->status,
            &gnrmc->latitude,
            &gnrmc->ns,
            &gnrmc->longitude,
            &gnrmc->ew,
            &gnrmc->ground_speed,
            &gnrmc->course_over_ground,
            &gnrmc->date);
    return ;
}
// REGISTER_MESSAGE_PARSER(GNRMC, 0, &gnrmc_data, gnrmc_data_parse);

static void gnatt_data_parse(void *data, const char *payload, size_t len)
{
    GNATTData *gnatt = (GNATTData*)data;

    sscanf((const char *)payload, "%[^,],%lf,%c,%lf,%lf,%lf,%lf,%lf,%lf",
            gnatt->talker_id,
            &gnatt->time,
            &gnatt->status,
            &gnatt->roll,
            &gnatt->pitch,
            &gnatt->heading,
            &gnatt->roll_uncertainty,
            &gnatt->pitch_uncertainty,
            &gnatt->heading_uncertainty);
    return ;
}   
// REGISTER_MESSAGE_PARSER(GNATT, 0, &gnatt_data, gnatt_data_parse);

static void pgnss3_data_parse(void *data, const char *payload, size_t len)
{
    PGNSS3Data *pgnss3 = (PGNSS3Data*)data;

    sscanf((const char *)payload, "%[^,],%hhu,%hu,%lf,%lf,%f,%lf,%hhu,%hhu,%hu,%hu",
            pgnss3->message_id,
            &pgnss3->subid,
            &pgnss3->svid,
            &pgnss3->tow,
            &pgnss3->pseudorange,
            &pgnss3->carrier_doppler,
            &pgnss3->carrier_phase,
            &pgnss3->glonass_freq_num,
            &pgnss3->cn0,
            &pgnss3->measurement_state,
            &pgnss3->reserved);
    return ;
}
// REGISTER_MESSAGE_PARSER(PGNSS, 3, &pgnss3_data, pgnss3_data_parse);

static void pgnss4_data_parse(void *data, const char *payload, size_t len)
{
    PGNSS4Data *pgnss4 = (PGNSS4Data*)data;

    sscanf((const char *)payload, "%[^,],%hhu,%hu,%lf,%lf,%lf,%hhu,%hhu,%hhu,%f",
            pgnss4->message_id,
            &pgnss4->subid,
            &pgnss4->svid,
            &pgnss4->tow,
            &pgnss4->pseudorange,
            &pgnss4->carrier_phase,
            &pgnss4->loss_of_lock,
            &pgnss4->cn0,
            &pgnss4->reserved1,
            &pgnss4->reserved2);
    return ;
}
// REGISTER_MESSAGE_PARSER(PGNSS, 4, &pgnss4_data, pgnss4_data_parse);

static void pgnss5_data_parse(void *data, const char *payload, size_t len)
{
    PGNSS5Data *pgnss5 = (PGNSS5Data*)data;

    sscanf((const char *)payload, "%[^,],%hhu,%hu,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%hhu,%hhu,%lf,%hhu,%hhu",
            pgnss5->message_id,
            &pgnss5->subid,
            &pgnss5->svid,
            &pgnss5->tow,
            &pgnss5->pos_x,
            &pgnss5->pos_y,
            &pgnss5->pos_z,
            &pgnss5->vel_x,
            &pgnss5->vel_y,
            &pgnss5->vel_z,
            &pgnss5->reserved1,
            &pgnss5->reserved2,
            &pgnss5->sv_clock_bias,
            &pgnss5->sv_state_valid,
            &pgnss5->reserved);
    return ;
}
// REGISTER_MESSAGE_PARSER(PGNSS, 5, &pgnss5_data, pgnss5_data_parse);

static void pgnss9_data_parse(void *data, const char *payload, size_t len)
{
    PGNSS9Data *pgnss9 = (PGNSS9Data*)data;

    sscanf((const char *)payload, "%[^,],%hhu,%hu,%lf,%lf,%lf,%lf",
            pgnss9->message_id,
            &pgnss9->subid,
            &pgnss9->station_id,
            &pgnss9->ecef_x,
            &pgnss9->ecef_y,
            &pgnss9->ecef_z,
            &pgnss9->reserved);
    return ;
}
// REGISTER_MESSAGE_PARSER(PGNSS, 9, &pgnss9_data, pgnss9_data_parse);

static void psnsr21_data_parse(void *data, const char *payload, size_t len)
{
    PSNSR21Data *psnsr21 = (PSNSR21Data*)data;
    const char *delimiters = ",";
    char *save_ptr;
    char *buf = malloc(len + 1);
    strncpy(buf, payload, len);
    buf[len] = '\0';


    char *token = strtok_r(buf, delimiters, &save_ptr);
    if (token) {
        strncpy(psnsr21->message_id, token, sizeof(psnsr21->message_id) - 1);
        psnsr21->message_id[sizeof(psnsr21->message_id) - 1] = '\0';
    }
    token = strtok_r(NULL, delimiters, &save_ptr);
    if (token) {
        psnsr21->sensortype = (uint8_t)atoi(token);
    }
    token = strtok_r(NULL, delimiters, &save_ptr);
    if (token) {
        psnsr21->num_sets = (uint8_t)atoi(token);
    }
    // sscanf(payload, "%[^,],%hhu,%hhu",
    //         psnsr21->message_id,
    //         &psnsr21->sensortype,
    //         &psnsr21->num_sets);

    for (uint8_t num = 0; num < psnsr21->num_sets; num++) {
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->time_tag[num] = (uint32_t)atoi(token);
        }
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->gyro_x[num] = (int32_t)atoi(token);
        }
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->gyro_y[num] = (int32_t)atoi(token);
        }
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->gyro_z[num] = (int32_t)atoi(token);
        }
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->accel_x[num] = (int32_t)atoi(token);
        }
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->accel_y[num] = (int32_t)atoi(token);
        }
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->accel_z[num] = (int32_t)atoi(token);
        }
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->odo[num] = (uint32_t)atoi(token);
        }
        token = strtok_r(NULL, delimiters, &save_ptr);
        if (token) {
            psnsr21->reverse[num] = (uint8_t)atoi(token);
        }
    }

    free(buf);
    return ;
}
// REGISTER_MESSAGE_PARSER(PSNSR, 21, &psnsr21_data, psnsr21_data_parse);

static void psnsr23_data_parse(void *data, const char *payload, size_t len)
{
    PSNSR23Data *psnsr23 = (PSNSR23Data*)data;

    sscanf(payload, "%[^,],%hhu,%u,%d,%d,%d,%d,%d,%d",
            psnsr23->message_id,
            &psnsr23->sensortype,
            &psnsr23->time_tag,
            &psnsr23->gyro_x,
            &psnsr23->gyro_y,
            &psnsr23->gyro_z,
            &psnsr23->accel_x,
            &psnsr23->accel_y,
            &psnsr23->accel_z);
    return ;
}   
// REGISTER_MESSAGE_PARSER(PSNSR, 23, &psnsr23_data, psnsr23_data_parse);

static void pblkend_data_parse(void *data, const char *payload, size_t len)
{
    PBLKENDData *pblkend = (PBLKENDData*)data;

    printf("pblkend_data_parse %s    len  %ld\n", payload, len);
    sscanf((const char *)payload, "%[^,],%hhu,%u",
            pblkend->message_id,
            &pblkend->end_flag,
            &pblkend->unix_time);
    return ;
}
// REGISTER_MESSAGE_PARSER(PBEND, 255, &pblkend_data, pblkend_data_parse);
#endif

void message_parser_entry(const char *line)
{
    struct MessageParser *start = &__start_message_parsers;
    struct MessageParser *end = &__stop_message_parsers;
    for (; start != end; start++) {
        if (start != NULL && start->func != NULL) {
 //           printf("p->hdr.msg_id %s--%ld\n", start->hdr.msg_id, strlen(start->hdr.msg_id));
            if (strncmp(line, start->hdr.msg_id, strlen(start->hdr.msg_id)) == 0) {
                start->func(start->data, line, strlen(line));
                break;
            }
        }
    }
}

void laneTo_read_nav_data(LaneToCtx *ctx) 
{
    char buffer[4*1024];
    SerialPort *serial = NULL;
    char *end = NULL;
    char *start = NULL;
    ssize_t bytes_read = 0;
    char *token = NULL;
    char *rest = NULL;
    
    if (ctx->running == false) {
        printf("laneTo not running\n");
        return;
    }
    if (ctx->uart) {
        serial = &ctx->uart->base;
        bytes_read = serial->ops->read(serial, buffer, sizeof(buffer));
    } else {
        bytes_read = read(ctx->sockfd, buffer, sizeof(buffer));
    }
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        start = strstr(buffer, SG_MSG_ID);
        // end = strstr(buffer, PBLKEND_MSG_ID);
        end = strstr(buffer, GNGGA_MSG_ID);
        // printf("start :%s, end: %s, strlen: %ld\n",  start, end, end - start);
        if (start != NULL && end != NULL && end > start) {
            char *data = strndup(start, end - start);
            if (data) {
                rest = data;
                token = strtok_r(data, "$", &rest);
                while (token != NULL) {
                    // printf("%s---%ld\n", token, strlen(token));
                    message_parser_entry(token);
                    token = strtok_r(NULL, "$", &rest);
                }
                free(data);
            }
        } 
    }

    return ;
}

int init_shm_data_sock(LaneToCtx *ctx)
{
    int sockfd = -1;
    struct sockaddr_in server_addr;

    ctx->sockfd = -1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed.");
        return -1;
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SHM_DATA_SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SHM_DATA_SERVER_IP);

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Shared data sock connection failed.\n");
        close(sockfd);
        return -1;
    }

    // 发送命令
    const char *command = "?WATCH={\"enable\":true,\"raw\":3}\n";
    if (send(sockfd, command, strlen(command), 0) < 0) {
        close(sockfd);
        return -1;
    }

    ctx->sockfd = sockfd;
    printf("shm data sock init success.\n");
    return 0;
}

int laneTo_init(LaneToCtx *ctx)
{   
    UartPort *laneTo_port = NULL;

    if (ctx == NULL) {
        return -1;
    }

    memset(ctx, 0, sizeof(LaneToCtx));
    ctx->uart = NULL;
    int ret = init_shm_data_sock(ctx);
    if (ret) {
        SerialPortInfo laneto_port_info = {
            .speed = 115200, 
            .data_bits = 8, 
            .stop_bits = 1, 
            .parity = 'N', 
            .fctl = 0
        };
        laneTo_port = uart_port_create();
        if (laneTo_port == NULL) {
            return -1;
        }

        int ret = laneTo_port->base.ops->open(&laneTo_port->base, LANETO_DEV_NAME);
        if (ret) {
            return -1;
        }
        ret = laneTo_port->base.ops->configure(&laneTo_port->base, &laneto_port_info);
        if (ret) {
            return -1;
        }
        ctx->uart = laneTo_port;
        // printf("fd = %d\n", ctx->uart->base.fd);
    }
    ctx->running = true;
    printf("laneTo_init ok!\n");
    return 0;
}

void laneTo_uninit(LaneToCtx *ctx)
{
    UartPort *laneTo_port = NULL;
    if (ctx == NULL) {
        return ;
    }

    printf("laneTo_uninit.\n");
    ctx->running = false;
    if (ctx->uart) {
        laneTo_port = ctx->uart;
        laneTo_port->base.ops->close(&laneTo_port->base);
    }
    if (ctx->sockfd != -1) {
         close(ctx->sockfd);
    }
    free(laneTo_port);
    // free(ctx);
}

