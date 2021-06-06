#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
using namespace std;

#pragma comment(lib , "ws2_32.lib")

#define BUFSIZE 4096 /*缓冲区大小*/

int main()
{
    char Buffer[BUFSIZE];
    int ret;

    unsigned short port;
    struct hostent* host = NULL;

    //打开网络库
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
        printf("Winsock   初始化失败!\n");
        return 0;
    }

    /*创建Socket*/
    SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET) {
        printf("创建socket() 失败: %d\n", WSAGetLastError());
        return 0;
    }

    //指定服务器地址
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    //与服务器建立连接
    if (connect(sClient, (struct sockaddr*)&server,
        sizeof(server)) == SOCKET_ERROR) {
        printf("connect() 失败: %d\n", WSAGetLastError());
        return 0;
    }

    //发送、接收消息
    while(true) 
    {
        //发送消息
        printf("client>>");
        string data;
        cin >> data;
        const char* sendData2;
        sendData2 = data.c_str(); 
        ret=send(sClient, sendData2, strlen(sendData2), 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) {
            printf("send() 失败: %d\n", WSAGetLastError());
            break;
        }

       
        //接收消息
        ret = recv(sClient, Buffer, BUFSIZE, 0);
        if (ret == 0) {
            break;
        }
        else if (ret == SOCKET_ERROR) 
        {
            printf("recv() error: %d\n", WSAGetLastError());
            break;
        }
        Buffer[ret] = '\0';
        printf("server>>%s\n", Buffer);

    }
    //用完，关闭socket句柄(文件描述符)
    closesocket(sClient);
    WSACleanup();    //清理
    return 0;
}
