#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


int main() 
{

    char resp[] = "采集单元:11,11;控制单元:22,22;通信单元:33,33;设备单元:44,44;";
    int i = 0;
    char *token = strtok(resp, ";"); 
    while (token != NULL) 
    {
        i++;
        printf("Token: %s, i %d\n", token, i);
        token = strtok(NULL, ";");
    }

    printf("resp  %s\n", resp);

    return 0;    
}

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2947
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    // 发送命令
    const char *command = "?WATCH={\"enable\":true,\"json\":true}\n";
    if (send(sockfd, command, strlen(command), 0) < 0) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // 读取数据
    while (1) {
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("recv");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("GPS Data: %s", buffer);
    }

    // 关闭套接字
    close(sockfd);
    return 0;
}