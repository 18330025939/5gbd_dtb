#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/var/run/gpsd.sock"
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // 创建套接字
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    // 发送命令
    const char *command = "?WATCH={\"enable\":true,\"raw\":3}\n";
    if (write(sockfd, command, strlen(command)) < 0) {
        perror("write");
        close(sockfd);
        exit(1);
    }

    // 每秒读取一次数据
    while (1) {
        int bytes_received = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (bytes_received < 0) {
            perror("read");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("GPS Data: %s", buffer);

        // 等待 1 秒
        sleep(1);
    }

    // 关闭套接字
    close(sockfd);
    return 0;
}