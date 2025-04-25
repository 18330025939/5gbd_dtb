#ifndef __FX650_H
#define __FX650_H

#define AT_TIMEOUT_MS   1000
#define URC_BUFFER_SIZE 256
#define AT_MAX_RESPONSE_LEN 1024

#define FX650_PID  "A05"
#define FX650_VID  "2CB7"

#define FX650_DEV_NAME "/dev/ttyUSB1"

// 模组信息相关定义
#define AT_CGMI "+CGMI"          // 获取制造商 ID
#define AT_GMI "+GMI"            // 获取制造商 ID
#define AT_CGMM "+CGMM"          // 获取型号 ID
#define AT_GMM "+GMM"            // 获取型号 ID
#define AT_CGMR "+CGMR"          // 获取软件修订版本
#define AT_GMR "+GMR"            // 获取软件修订版本
#define AT_CGSN "+CGSN"          // 获取产品串号识别
#define AT_GSN "+GSN"            // 获取产品串号识别
#define AT_CFSN "+CFSN"          // 获取工厂串号
#define AT_CIMI "+CIMI"          // 获取国际移动用户身份（IMSI）
#define AT_CNUM "+CNUM"          // 获取 MSISDN 信息
#define AT_CCID "+CCID"          // 获取集成电路卡识别码
// 模组控制与状态相关定义
#define AT_CFUN "+CFUN"          // 设置模组功能级别
#define AT_CPWROFF "+CPWROFF"    // 控制模组电源开关
#define AT_CSCLK "+CSCLK"        // 配置是否进入睡眠模式
#define AT_CPAS "+CPAS"          // 获取模组当前活动状态
// 通话控制相关定义
#define AT_D "+D"                // 拨号命令
#define AT_DL "+DL"              // 拨打最后拨号的号码
#define AT_H "+H"                // 挂断电话
#define AT_A "+A"                // 接听来电
#define AT_CRC "+CRC"            // 控制通话状态消息的显示
#define AT_CLIP "+CLIP"          // 呼叫线路识别
// 系统日期与时间相关定义
#define AT_CCLK "+CCLK"          // 读取或设置系统当前日期和时间
#define AT_CTZU "+CTZU"          // 启用或禁用自动时间更新
// 短信相关定义
#define AT_CSCS "+CSCS"          // 选择终端字符集
#define AT_CSMS "+CSMS"          // 选择短信服务
#define AT_CPMS "+CPMS"          // 选择短信存储区域
#define AT_CMGF "+CMGF"          // 设置短信格式
#define AT_CSCA "+CSCA"          // 设置短信中心地址
#define AT_CSMP "+CSMP"          // 设置短信文本模式参数
#define AT_CSDH "+CSDH"          // 显示短信文本模式参数
#define AT_CNMI "+CNMI"          // 新短信指示
#define AT_CMGL "+CMGL"          // 列出短信
#define AT_CMGR "+CMGR"          // 读取短信
#define AT_CMGS "+CMGS"          // 发送短信
#define AT_CMSS "+CMSS"          // 从存储发送短信
#define AT_CMGW "+CMGW"          // 写短信到存储
#define AT_CMGD "+CMGD"          // 删除短信
// 接入与安全相关定义
#define AT_CPIN "+CPIN"          // SIM 卡解锁
#define AT_CWPIN "+CPIN"         // SIM 卡解锁
#define AT_CWOPIN "+CPIN"        // SIM 卡解锁
#define AT_CLCK "+CLCK"          // 设备锁定
#define AT_COPS "+COPS"          // 网络选择
#define AT_CPLS "+CPLS"          // 选择 PLMN 列表
// 网络相关定义
#define AT_CSQ "+CSQ"            // 获取信号强度
#define AT_CESQ "+CESQ"          // 获取扩展信号质量
#define AT_CREG "+CREG"          // 网络注册状态
#define AT_CGREG "+CGREG"        // GPRS 网络注册状态
#define AT_CEREG "+CEREG"        // EPS 网络注册状态
#define AT_C5GEREG "+C5GEREG"    // NR 网络注册状态
#define AT_GTRAT "+GTRAT"        // 选择无线接入技术
#define AT_GTACT "+GTACT"        // 选择 RAT 和 BAND
// GPRS 相关定义
#define AT_CGCLASS "+CGCLASS"    // 设置 GPRS 移动台类
#define AT_CGDCONT "+CGDCONT"    // 定义 PDP 上下文
#define AT_CGQMIN "+CGQMIN"      // 设置 QoS 配置（最小可接受）
#define AT_CGQREQ "+CGQREQ"      // 设置 QoS 配置（请求）
#define AT_CGATT "+CGATT"        // GPRS 附着或分离
#define AT_CGACT "+CGACT"        // PDP 上下文激活或去激活
#define AT_CGPADDR "+CGPADDR"    // 获取 GPRS 地址
#define AT_GTDNS "+GTDNS"        // 获取 DNS 地址
#define AT_CGEQMIN "+CGEQMIN"    // 3G QoS 配置（最小可接受）
#define AT_CGEQREQ "+CGEQREQ"    // 3G QoS 配置（请求）
#define AT_CGEQNEG "+CGEQNEG"    // 3G QoS 配置（协商）
#define AT_CGDATA "+CGDATA"      // 进入数据状态
#define AT_CGDSCONT "+CGDSCONT"  // 定义次级 PDP 上下文
#define AT_CGEREP "+CGEREP"      // 包数据域事件报告
#define AT_CGTFT "+CGTFT"        // 流量流模板
#define AT_CSCON "+CSCON"        // 信令连接状态
#define AT_CGCONTRDP "+CGCONTRDP" // PDP 上下文读取动态参数
#define AT_CGSCONTRDP "+CGSCONTRDP" // 次级 PDP 上下文读取动态参数
#define AT_CGTFTRDP "+CGTFTRDP"  // 流量流模板读取动态参数
// Fibocom 特有命令相关定义
#define AT_GTDIV "+GTDIV"        // 设置 3G 多样性
#define AT_GTUSBMODE "+GTUSBMODE" // 设置 USB 配置文件
#define AT_GTRNDIS "+GTRNDIS"    // RNDIS 配置
#define AT_GTAUTOCONNECT "+GTAUTOCONNECT" // 自动连接
#define AT_GTFMODE "+GTFMODE"    // 硬件引脚控制飞行模式
#define AT_MMAD "+MMAD"          // 查询 ADC 通道电压值
#define AT_GTIPPASS "+GTIPPASS"  // 启用 IP 透明传输
#define AT_GTPING "+GTPING"      // 检查数据业务连接状态
#define AT_GTSTATIS "+GTSTATIS"  // 显示传输和接收速率及总字节数
#define AT_GTMAPVLAN "+GTMAPVLAN" // 映射 VLAN ID
#define AT_GTMPDN "+GTMPDN"      // 启用 VLAN 多个 PDN
// 硬件信息相关定义
#define AT_CLK32K "+CLK32K"      // 32KHz 时钟输出
#define AT_CLK26M "+CLK26M"      // 26MHz 时钟输出
// 错误处理与错误码相关定义
#define AT_CMEE "+CMEE"          // 报告移动设备错误
#define AT_CEER "+CEER"          // 扩展错误报告


// CME 错误码
typedef enum {
    CME_ERROR_PHONE_FAILURE = 0,
    CME_ERROR_NO_CONNECTION_TO_PHONE = 1,
    CME_ERROR_PHONE_LINK_RESERVED = 2,
    CME_ERROR_OPERATION_NOT_ALLOWED = 3,
    CME_ERROR_OPERATION_NOT_SUPPORTED = 4,
    CME_ERROR_PH_SIM_PIN_REQUIRED = 5,
    CME_ERROR_PH_FSIM_PIN_REQUIRED = 6,
    CME_ERROR_PH_FSIM_PUK_REQUIRED = 7,
    CME_ERROR_SIM_NOT_INSERTED = 10,
    CME_ERROR_SIM_PIN_REQUIRED = 11,
    CME_ERROR_SIM_PUK_REQUIRED = 12,
    CME_ERROR_SIM_FAILURE = 13,
    CME_ERROR_SIM_BUSY = 14,
    CME_ERROR_SIM_WRONG = 15,
    CME_ERROR_INCORRECT_PASSWORD = 16,
    CME_ERROR_SIM_PIN2_REQUIRED = 17,
    CME_ERROR_SIM_PUK2_REQUIRED = 18,
    CME_ERROR_INCORRECT_PUK1 = 19,
    CME_ERROR_MEMORY_FULL = 20,
    CME_ERROR_INVALID_INDEX = 21,
    CME_ERROR_NOT_FOUND = 22,
    CME_ERROR_MEMORY_FAILURE = 23,
    CME_ERROR_TEXT_STRING_TOO_LONG = 24,
    CME_ERROR_INVALID_CHARACTERS_IN_TEXT_STRING = 25,
    CME_ERROR_DIAL_STRING_TOO_LONG = 26,
    CME_ERROR_INVALID_CHARACTERS_IN_DIAL_STRING = 27,
    CME_ERROR_NO_NETWORK_SERVICE = 30,
    CME_ERROR_NETWORK_TIMEOUT = 31,
    CME_ERROR_NETWORK_NOT_ALLOWED = 32,
    CME_ERROR_NETWORK_PERSONALIZATION_PIN_REQUIRED = 40,
    CME_ERROR_NETWORK_PERSONALIZATION_PUK_REQUIRED = 41,
    CME_ERROR_NETWORK_SUBSET_PERSONALIZATION_PIN_REQUIRED = 42,
    CME_ERROR_NETWORK_SUBSET_PERSONALIZATION_PUK_REQUIRED = 43,
    CME_ERROR_SERVICE_PROVIDER_PERSONALIZATION_PIN_REQUIRED = 44,
    CME_ERROR_SERVICE_PROVIDER_PERSONALIZATION_PUK_REQUIRED = 45,
    CME_ERROR_CORPORATE_PERSONALIZATION_PIN_REQUIRED = 46,
    CME_ERROR_CORPORATE_PERSONALIZATION_PUK_REQUIRED = 47,
    CME_ERROR_HIDDEN_KEY_REQUIRED = 48,
    CME_ERROR_EAP_METHOD_NOT_SUPPORTED = 49,
    CME_ERROR_INCORRECT_PARAMETERS = 50,
    CME_ERROR_UNKNOWN = 100,
    // 更多错误码...
} CMEErrorCode;

// CMS 错误码
typedef enum {
    CMS_ERROR_UNASSIGNED_NUMBER = 1,
    CMS_ERROR_OPERATOR_DETERMINED_BARRING = 8,
    CMS_ERROR_CALL_BARRED = 10,
    CMS_ERROR_NETWORK_FAILURE = 17,
    CMS_ERROR_SHORT_MESSAGE_TRANSFER_REJECTED = 21,
    CMS_ERROR_MEMORY_CAPACITY_EXCEEDED = 22,
    CMS_ERROR_DESTINATION_OUT_OF_SERVICE = 27,
    CMS_ERROR_UNIDENTIFIED_SUBSCRIBER = 28,
    CMS_ERROR_FACILITY_REJECTED = 29,
    CMS_ERROR_UNKNOWN_SUBSCRIBER = 30,
    CMS_ERROR_NETWORK_OUT_OF_ORDER = 38,
    CMS_ERROR_TEMPORARY_FAILURE = 41,
    CMS_ERROR_CONGESTION = 42,
    CMS_ERROR_RESOURCES_UNAVAILABLE = 47,
    CMS_ERROR_REQUESTED_FACILITY_NOT_SUBSCRIBED = 50,
    CMS_ERROR_SERVICE_OR_OPTION_NOT_IMPLEMENTED = 69,
    // 更多错误码...
} CMSErrorCode;


// 模组信息结构体
typedef struct {
    char manufacturer[64];
    char model[64];
    char revision[64];
    char imei[16];
    char imsi[16];
    char msisdn[16];
    char ccid[16];
} ModemInfoStruct;

// 网络注册状态结构体
typedef struct {
    int mode;
    int status;
    int lac;
    int ci;
    int act;
} NetworkRegistrationStatus;

// 短信结构体
typedef struct {
    int index;
    char number[32];
    char message[256];
    int status;
    int length;
} SMSMessage;

// GPRS PDP 上下文结构体
typedef struct {
    int cid;
    char pdp_type[16];
    char apn[64];
    char pdp_addr[32];
} GPRSPDPContext;

// 网络信号质量结构体
typedef struct {
    int rssi;
    int ber;
    int ss_rsrp;
    int ss_rsrq;
    int ss_sinr;
} NetworkSignalQuality;

// 位置信息结构体
typedef struct {
    int pas;
    int stat;
} LocationInfo;

// 通话状态结构体
typedef struct {
    int status;
    int type;
    char number[32];
} CallStatus;

// 短信中心地址结构体
typedef struct {
    char sca[32];
    int tosca;
} SMSCenterAddress;

// 短信参数结构体
typedef struct {
    int mode;
    int format;
    char charset[16];
} SMSParameters;

// GPRS 附着状态结构体
typedef struct {
    int state;
} GPRSAttachmentStatus;

// PDP 上下文激活状态结构体
typedef struct {
    int cid;
    int state;
} PDPContextActivationStatus;

// DNS 信息结构体
typedef struct {
    char primary_dns[16];
    char secondary_dns[16];
} DNSInfo;

// 位置区信息结构体
typedef struct {
    int lac;
    int ci;
} LocationAreaInfo;

// 小区信息结构体
typedef struct {
    int tac;
    int ci;
    int ss_rsrp;
    int ss_rsrq;
    int ss_sinr;
} CellInfo;

// RAT 信息结构体
typedef struct {
    int rat;
    int band;
} RATInfo;

// 网络类型结构体
typedef struct {
    char network_type[16];
} NetworkType;

// 错误报告结构体
typedef struct {
    int category;
    int cause;
    char description[64];
} ErrorReport;

typedef struct {
    char at_buffer[256];
    char resp_buffer[1024];
    size_t resp_len;
} AtCommandContext;

// 网络状态结构体
typedef struct {
    int rssi;          // 信号强度
    unsigned char reg_status; // 注册状态
    char lac[8];       // 位置区码
    char ci[8];        // 小区ID
} NetworkStatus;

typedef struct {
    char apn[64];
    char username[32];
    char password[32];
} APNConfig;

// 回调函数类型定义
// typedef void (*DataReceivedCallback)(uint8_t* data, size_t len);
// typedef void (*NetworkEventCallback)(int event);

typedef struct st_Fx650Ctx {
    UartPort *uart;
    char *net_name;
    // NetworkStatus net_status;
    // APNConfig apn_config;
    // DataReceivedCallback data_cb;
    // NetworkEventCallback event_cb;
} Fx650Ctx;

// AT指令响应解析状态机
typedef enum {
    AT_STATE_IDLE,
    AT_STATE_WAIT_OK,
    AT_STATE_WAIT_CME_ERROR,
    AT_STATE_PROCESS_DATA
} AT_ParseState;

// 错误码定义
typedef enum {
    FX650_OK = 0,
    FX650_ERR_INIT,
    FX650_ERR_AT_TIMEOUT,
    FX650_ERR_AT_ERROR,
    FX650_ERR_SIM_NOT_READY,
    FX650_ERR_APN_NOT_READY,
    FX650_ERR_NET_REG,
    FX650_ERR_PDP_ACTIVATE,
    FX650_ERR_SOCKET,
    FX650_ERR_TX
} FX650_Error;

FX650_Error fx650_connect_network(Fx650Ctx* ctx);
FX650_Error fx650_init(Fx650Ctx* ctx); 
void fx650_uninit(Fx650Ctx* ctx);
#endif