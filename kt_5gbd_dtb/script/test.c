#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

// 定义 gpsd 的共享内存结构体
struct gps_fix {
    double latitude;       // 纬度
    double longitude;      // 经度
    double altitude;       // 海拔
    double speed;          // 速度
    double climb;          // 上升速度
    double track;          // 航向
    double magvar;         // 磁偏角
    double time;           // 时间
    double ept;            // 时间误差
    double eph;            // 水平位置误差
    double epv;            // 垂直位置误差
    double ehdop;          // 水平精度因子
    double evdop;          // 垂直精度因子
    double epdop;          // 位置精度因子
    int mode;              // 模式
    int satellites;        // 卫星数量
    char status[32];       // 状态
    char device[32];       // 设备名
};

int main() {
    key_t key = 0x47505344;  // GPS 数据的共享内存键值
    int shmid = shmget(key, sizeof(struct gps_fix), 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    struct gps_fix *data = (struct gps_fix *)shmat(shmid, NULL, 0);
    if (data == (struct gps_fix *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("Shared memory data:\n");
    printf("Latitude: %f\n", data->latitude);
    printf("Longitude: %f\n", data->longitude);
    printf("Altitude: %f\n", data->altitude);
    printf("Speed: %f\n", data->speed);
    printf("Climb: %f\n", data->climb);
    printf("Track: %f\n", data->track);
    printf("Magvar: %f\n", data->magvar);
    printf("Time: %f\n", data->time);
    printf("EPT: %f\n", data->ept);
    printf("EPH: %f\n", data->eph);
    printf("EPV: %f\n", data->epv);
    printf("EHDOP: %f\n", data->ehdop);
    printf("EVDOP: %f\n", data->evdop);
    printf("EPDOP: %f\n", data->epdop);
    printf("Mode: %d\n", data->mode);
    printf("Satellites: %d\n", data->satellites);
    printf("Status: %s\n", data->status);
    printf("Device: %s\n", data->device);

    shmdt(data);
    return 0;
}