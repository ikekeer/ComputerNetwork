#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
using namespace std;

#pragma comment(lib , "ws2_32.lib")

#define BUFSIZE 4096 /*��������С*/

int main()
{
    char Buffer[BUFSIZE];
    int ret;

    unsigned short port;
    struct hostent* host = NULL;

    //�������
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
        printf("Winsock   ��ʼ��ʧ��!\n");
        return 0;
    }

    /*����Socket*/
    SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET) {
        printf("����socket() ʧ��: %d\n", WSAGetLastError());
        return 0;
    }

    //ָ����������ַ
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    //���������������
    if (connect(sClient, (struct sockaddr*)&server,
        sizeof(server)) == SOCKET_ERROR) {
        printf("connect() ʧ��: %d\n", WSAGetLastError());
        return 0;
    }

    //���͡�������Ϣ
    while(true) 
    {
        //������Ϣ
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
            printf("send() ʧ��: %d\n", WSAGetLastError());
            break;
        }

       
        //������Ϣ
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
    //���꣬�ر�socket���(�ļ�������)
    closesocket(sClient);
    WSACleanup();    //����
    return 0;
}
