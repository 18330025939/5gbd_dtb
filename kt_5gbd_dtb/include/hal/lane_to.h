#ifndef __LANE_TO_H
#define __LANE_TO_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include "serial.h"

#define LANETO_DEV_NAME "/dev/ttys4"

#pragma pack(push, 1)
typedef struct {
    uint8_t sync1;        // 固定为0x50
    uint8_t sync2;        // 固定为0x42
    uint8_t msg_id;       // 命令ID (0xE0 ~ 0xFF)
    uint16_t payload_len; // Little-Endian
    uint8_t payload[];    // 可变长度负载
} BinaryCommandHeader;
#pragma pack(pop)

// 常用命令ID宏定义
#define CMD_REVERT_MFG_SETTING       0xE0
#define CMD_POLL_FW_VERSION          0xE3
#define CMD_SET_NAV_RATE             0xE4
#define CMD_FORCED_RESET             0xE6
#define CMD_DEBUG_MESSAGE_CONTROL    0xE7
#define CMD_BAUD_RATE_SETTING        0xEA


#define SUBID_SG_NAVIGATION_RESULT             1
#define SUBID_ROVER_GNSS_CHANNEL_DATA          3
#define SUBID_BASE_GNSS_MEASUREMENT_DATA       4
#define SUBID_GNSS_SV_STATES                   5
#define SUBID_REFERENCE_STATION_ID_POSITION    9
#define SUBID_SENSOR_DATA                      21
#define SUBID_SENEOR_RAW_DATA                  23
#define SUBID_BLOCK_END                        255


// 设置导航速率 (CMD_SET_NAV_RATE)
typedef enum {
    NAV_RATE_1HZ = 0,
    NAV_RATE_10HZ = 1,
    NAV_RATE_20HZ = 2,
    NAV_RATE_50HZ = 3
} NavRate;

// 设置波特率 (CMD_BAUD_RATE_SETTING)
typedef enum {
    BAUD_115200 = 0,
    BAUD_38400 = 1,
    BAUD_19200 = 2,
    BAUD_9600 = 3
} BaudRate;

#define SG_MSG_ID        "$PBSOL"
#define GNGGA_MSG_ID     "GNGGA"
#define GNRMC_MSG_ID     "GNRMC"
#define GNATT_MSG_ID     "GNATT"
#define PGNSS3_MSG_ID    "PGNSS"
#define PGNSS4_MSG_ID    "PGNSS"
#define PGNSS5_MSG_ID    "PGNSS"
#define PGNSS9_MSG_ID    "PGNSS"
#define PSNSR21_MSG_ID   "PSNSR"
#define PSNSR23_MSG_ID   "PSNSR"
#define PBLKEND_MSG_ID   "$PBEND"

#define SG_MSG_SUBID        1
#define PGNSS3_MSG_SUBID    3
#define PGNSS4_MSG_SUBID    4
#define PGNSS5_MSG_SUBID    5
#define PGNSS9_MSG_SUBID    9
#define PSNSR21_MSG_SUBID   21
#define PSNSR23_MSG_SUBID   23
#define PBLKEND_MSG_SUBID   255

#pragma pack(push, 1)
/* Navigation results in SG defined format */
typedef struct {
    char message_id[7];              // 消息ID，例如 "PBSOL"
    uint8_t subid;                   // 子消息ID
    uint16_t utc_year;               // UTC时间-年份
    uint8_t utc_month;               // UTC时间-月份
    uint8_t utc_day;                 // UTC时间-日期
    uint8_t utc_hour;                // UTC时间-小时
    uint8_t utc_minutes;             // UTC时间-分钟
    uint16_t utc_millisecond;        // UTC时间-毫秒
    uint32_t sensor_time;            // 传感器时间（以毫秒为单位）
    uint16_t gps_week_number;        // GPS周数
    uint32_t tow;                    // 星期计数时间（以毫秒为单位）
    uint32_t nav_mode_info;          // 导航模式信息
                                     // 0x00: 无效；bit 0: 保留；bit 1: 仅GNSS；bit 2: 仅传感器；
                                     // bit 3: 组合解；bit 4~11: 保留；bit 12: 使用DGNSS；
                                     // bit 13: 使用RTK（固定解）；bit 14: 使用RTK（浮动解）；bit 15~31: 保留
    double latitude;                 // 纬度（十进制度数，8位小数）
    double longitude;                // 经度（十进制度数，8位小数）
    float altitude_ellipsoid;        // 椭球高度（米，3位小数）
    float altitude_msl;              // 平均海平面高度（米，3位小数）
    int32_t vn;                      // 北向速度（厘米/秒）
    int32_t ve;                      // 东向速度（厘米/秒）
    int32_t vd;                      // 垂直速度（厘米/秒）
    uint32_t ground_speed;           // 地面速度（厘米/秒）
    uint32_t traveled_distance;      // 行驶距离（厘米）
    int32_t roll;                    // 横滚角（度，缩放因子*100）
    int32_t pitch;                   // 俯仰角（度，缩放因子*100）
    int32_t heading;                 // 航向角（度，缩放因子*100，0~360，相对于真北，逆时针为正）
    int32_t reserved1;               // 保留字段1
    int32_t reserved2;               // 保留字段2
    uint16_t north_uncertainty;      // 北向不确定性（厘米）
    uint16_t east_uncertainty;       // 东向不确定性（厘米）
    uint16_t down_uncertainty;       // 垂直不确定性（厘米）
    uint16_t vn_uncertainty;         // 北向速度不确定性（厘米/秒）
    uint16_t ve_uncertainty;         // 东向速度不确定性（厘米/秒）
    uint16_t vd_uncertainty;         // 垂直速度不确定性（厘米/秒）
    uint16_t roll_uncertainty;       // 横滚角不确定性（度，缩放因子*100）
    uint16_t pitch_uncertainty;      // 俯仰角不确定性（度，缩放因子*100）
    uint16_t yaw_uncertainty;        // 航向角不确定性（度，缩放因子*100）
    int16_t misalign_angle_roll;     // 横滚角偏差（度，缩放因子*100）
    int16_t misalign_angle_pitch;    // 俯仰角偏差（度，缩放因子*100）
    int16_t misalign_angle_yaw;      // 航向角偏差（度，缩放因子*100）
    uint16_t reference_station_id;   // 参考站ID（若使用差分修正）
    uint8_t time_since_last_diff;    // 自上次差分修正以来的时间（秒）
    uint8_t reserved[2];              // 保留字段
} SGData;
SGData sg_data;

/* Global Navigation Satellite System Fix Data */
typedef struct {
    char talker_id[7];        // 例如 "GN" 表示 GNSS
    double time;              // UTC 时间
    double latitude;          // 纬度
    char ns;                  // 北南指示符（'N' 或 'S'）
    double longitude;         // 经度
    char ew;                  // 东西指示符（'E' 或 'W'）
    uint8_t fix_quality;      // 定位质量
    uint8_t num_satellites;   // 卫星数量
    float hdop;               // 水平精度因子
    float altitude_msl;       // 海拔高度（米）
    char altitude_unit;       // 高度单位（'M' 表示米）
    float geoid_separation;   // 地球分离（米）
    char geoid_unit;          // 地球分离单位（'M' 表示米）
    uint8_t checksum;         // 校验和
} GNGGAData;
GNGGAData gngga_data;

/* Recommended Minimum Navigation Information */
typedef struct {
    char talker_id[7];        // 例如 "GN" 表示 GNSS
    double time;              // UTC 时间
    char status;              // 定位状态（'A' 表示有效，'V' 表示无效）
    double latitude;          // 纬度
    char ns;                  // 北南指示符（'N' 或 'S'）
    double longitude;         // 经度
    char ew;                  // 东西指示符（'E' 或 'W'）
    float ground_speed;       // 地面速度（米/秒）
    float course_over_ground; // 航向角（度）
    uint32_t date;            // UTC日期
    uint8_t checksum;         // 校验和
} GNRMCData;
GNRMCData gnrmc_data;

/* 3D attitude message  */
typedef struct {
    char talker_id[7];          // 例如 "GN" 表示 GNSS
    double time;                // UTC 时间
    char status;                // 数据有效性状态
    double roll;                // 横滚角（度）
    double pitch;               // 俯仰角（度）
    double heading;             // 航向角（度）
    double roll_uncertainty;    // 横滚角不确定性（度）
    double pitch_uncertainty;   // 俯仰角不确定性（度）
    double heading_uncertainty; // 航向角不确定性（度）
    uint8_t checksum;           // 校验和
} GNATTData;
GNATTData gnatt_data;

/* Raw Rover GNSS Channel Data -- $PGNSS,3 */
typedef struct {
    char message_id[7];         // 消息 ID，例如 "$PGNSS"
    uint8_t subid;              // 子消息 ID，3 表示原始 GNSS 通道数据
    uint16_t svid;              // 卫星编号
    double tow;                 // 时间周（单位：秒）
    double pseudorange;         // 伪距（单位：米）
    float carrier_doppler;      // 载波 Doppler 频移（单位：Hz）
    double carrier_phase;       // 载波相位（单位：周）
    uint8_t glonass_freq_num;   // GLONASS 频率编号（仅适用于 GLONASS）
    uint8_t cn0;                // 信噪比（单位：dB-Hz）
    uint16_t measurement_state; // 测量状态
    uint16_t reserved;          // 保留字段
} PGNSS3Data;
PGNSS3Data pgnss3_data;

/* Raw Base GNSS Measurement Data --$PGNSS,4 */
typedef struct {
    char message_id[7];          // 消息 ID，例如 "$PGNSS"
    uint8_t subid;               // 子消息 ID，4 表示基础 GNSS 测量数据
    uint16_t svid;               // 卫星编号
    double tow;                  // 时间周（单位：秒）
    double pseudorange;          // 伪距（单位：米）
    double carrier_phase;        // 载波相位（单位：周）
    uint8_t loss_of_lock;        // 是否失去锁
    uint8_t cn0;                 // 信噪比（单位：dB-Hz）
    uint8_t reserved1;           // 保留字段
    float reserved2;             // 保留字段
} PGNSS4Data;
PGNSS4Data pgnss4_data;

/* GNSS SV States -- $PGNSS, 5 */
typedef struct {
    char message_id[7];      // 消息 ID，例如 "$PGNSS"
    uint8_t subid;           // 子消息 ID，5 表示 GNSS SV 状态
    uint16_t svid;           // 卫星编号
    double tow;              // 时间周（单位：秒）
    double pos_x;            // ECEF X 坐标（单位：米）
    double pos_y;            // ECEF Y 坐标（单位：米）
    double pos_z;            // ECEF Z 坐标（单位：米）
    double vel_x;            // ECEF X 速度（单位：米/秒）
    double vel_y;            // ECEF Y 速度（单位：米/秒）
    double vel_z;            // ECEF Z 速度（单位：米/秒）
    uint8_t reserved1;       // 保留字段
    uint8_t reserved2;       // 保留字段
    double sv_clock_bias;    // 卫星钟偏差（单位：米）
    uint8_t sv_state_valid;  // 卫星状态是否有效
    uint8_t reserved;        // 保留字段
} PGNSS5Data;
PGNSS5Data pgnss5_data;

/* Reference Station ID and Position -- $PGNSS, 9  */
typedef struct {
    char message_id[7];     // 消息 ID，例如 "$PGNSS"
    uint8_t subid;          // 子消息 ID，9 表示参考站 ID 和位置
    uint16_t station_id;    // 参考站 ID
    double ecef_x;          // ECEF X 坐标（单位：米）
    double ecef_y;          // ECEF Y 坐标（单位：米）
    double ecef_z;          // ECEF Z 坐标（单位：米）
    double reserved;        // 保留字段
} PGNSS9Data;
PGNSS9Data pgnss9_data;

/* Sensor Data -- $PSNSR,21  */
typedef struct {
    char message_id[7];       // 消息 ID，例如 "$PSNSR"
    uint8_t sensortype;       // 传感器类型，21 表示混合 DR 传感器
    uint8_t num_sets;         // 数据集数量
    // 以下字段重复 num_sets 次
    uint32_t time_tag[10];    // 时间戳（单位：毫秒）
    int32_t gyro_x[10];       // X 轴陀螺仪数据（单位：deg/s，缩放因子 *10^3）
    int32_t gyro_y[10];       // Y 轴陀螺仪数据（单位：deg/s，缩放因子 *10^3）
    int32_t gyro_z[10];       // Z 轴陀螺仪数据（单位：deg/s，缩放因子 *10^3）
    int32_t accel_x[10];      // X 轴加速度计数据（单位：m/s^2，缩放因子 *10^3）
    int32_t accel_y[10];      // Y 轴加速度计数据（单位：m/s^2，缩放因子 *10^3）
    int32_t accel_z[10];      // Z 轴加速度计数据（单位：m/s^2，缩放因子 *10^3）
    uint32_t odo[10];         // 速度计数据（单位：m/s，缩放因子 *10^3）
    uint8_t reverse[10];      // 方向标志（0 表示正向，1 表示反向）
} PSNSR21Data;
PSNSR21Data psnsr21_data;

/* Sensor Raw Data -- $PSNSR,23 */
typedef struct {
    char message_id[7];     // 消息 ID，例如 "$PSNSR"
    uint8_t sensortype;     // 传感器类型，23 表示原始传感器数据
    uint32_t time_tag;      // 时间戳（单位：毫秒）
    int32_t gyro_x;         // X 轴陀螺仪数据（单位：deg/s，缩放因子 *10^3）
    int32_t gyro_y;         // Y 轴陀螺仪数据（单位：deg/s，缩放因子 *10^3）
    int32_t gyro_z;         // Z 轴陀螺仪数据（单位：deg/s，缩放因子 *10^3）
    int32_t accel_x;        // X 轴加速度计数据（单位：m/s^2，缩放因子 *10^3）
    int32_t accel_y;        // Y 轴加速度计数据（单位：m/s^2，缩放因子 *10^3）
    int32_t accel_z;        // Z 轴加速度计数据（单位：m/s^2，缩放因子 *10^3）
} PSNSR23Data;
PSNSR23Data psnsr23_data;

/* Message block end -- $PBLKEND */
typedef struct {
    char message_id[8];    // 消息 ID，例如 "$PBLKEND"
    uint8_t end_flag;      // 结束标志，255 表示结束
    uint32_t unix_time;    // UNIX 时间戳（单位：秒），255 表示无效
} PBLKENDData;
PBLKENDData pblkend_data;
#pragma pack(pop)


typedef struct {
    const char *msg_id;     
    uint8_t sub_id;        
} MessageHeader;

typedef struct {
    MessageHeader hdr;
    void *data;
    void (*func)(void *data, const char *payload, size_t len);
} MessageParserEntry;

#define REGISTER_MESSAGE_PARSER(msg_id_str, sub_id_val, msg_data, parse_func)\
    static const MessageParserEntry parser_##msg_id_str##sub_id_val \
    __attribute__((used, section("message_parsers"))) = { \
        .hdr = { .msg_id = #msg_id_str, .sub_id = sub_id_val }, \
        .data = msg_data, \
        .func = parse_func \
    }

typedef struct {
    UartPort *uart;
    // char dev_name[20];
    // pthread_t recv_thread;
    // bool running;
} LaneToCtx;

UartPort *laneTo_port;

void laneTo_read_nav_data(LaneToCtx *ctx);
int laneTo_init(const char *uart_dev);
void laneTo_uninit(void);
#endif
