#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define DEFAULT_BUFFER 4096 //缓冲区大小
#define MAX 8
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <list>
#include <cmath>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

typedef struct Lience
{
	long lience = 0; //序列号
	int sum = 0;  //序列号允许使用人数
	int count = 0; //当前使用人数
}Lience;

list<Lience> lience_list;

//服务器接收消息
int  recv(SOCKET sock,char *Buffer)
{
	int ret, nLeft, idx;
	ret = recv(sock, Buffer, DEFAULT_BUFFER, 0);
	if (ret == 0)
		return 1;
	else if (ret == SOCKET_ERROR) {
		printf("recv() error: %d\n", WSAGetLastError());
		return 1;
	}
	Buffer[ret] = '\0';
	printf("client>> %s\n", Buffer);    //打印接收到的消息
	return 0;
}

//服务器发送消息
int send(SOCKET sock, const char* sendData)
{
	int ret;
	ret=send(sock, sendData, strlen(sendData), 0);
	if (ret == 0)
	{
		return 1;
	}
	else if (ret == SOCKET_ERROR) {
		printf("send() error: %d\n", WSAGetLastError());
		return 1;
	}
	printf("server>> %s\n", sendData);  
	return 0;
}

//购买许可证
char* generate_lience()
{
	char buffer[DEFAULT_BUFFER];
	Lience new_lience;
	new_lience.lience = lience_list.size();
	new_lience.sum = MAX;
	lience_list.push_back(new_lience);
	sprintf(buffer, "%010d", new_lience.lience);
	return buffer;
}

//运行，许可证使用人数加一
bool run(char *Buffer)
{
	bool allow = false;
	char buf[10];
	for (int i = 0;i < 10;i++)
		buf[i] = *(Buffer + i);
	long lience = atol(buf);
	for (list<Lience>::iterator i = lience_list.begin();i != lience_list.end();i++)
	{
		if ((*i).lience == lience && (*i).count < (*i).sum)
		{
			(*i).count++;
			return true;
		}
	}
	return false;
}

//退出，使许可证使用人数减一
void exit(char* Buffer)
{
	bool allow = false;
	char buf[10];
	for (int i = 0;i < 10;i++)
		buf[i] = *(Buffer + i);
	long lience = atol(buf);
	for (list<Lience>::iterator i = lience_list.begin();i != lience_list.end();i++)
	{
		if ((*i).lience == lience && (*i).count < (*i).sum)
		{
			(*i).count--;
		}
	}
}

//与客户机通信的线程函数
DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET sock = (SOCKET)lpParam;
	char Buffer[DEFAULT_BUFFER];
	char* lience=NULL;
	int ret;
	bool isRun = false;
	
	while (1) 
	{
		//接收来自客户机的消息
		ret=recv(sock,Buffer);
		if (ret == 1) break;

		//根据指令做出反应
		if (Buffer[0]=='b' && Buffer[1]=='u' && Buffer[2]=='y')  //如果是购买许可证服务
		{
			const char* sendData = "user ？";
			ret=send(sock, sendData);
			if (ret == 1) break;
			recv(sock, Buffer);
			if (ret == 1) break;
			ret = send(sock, generate_lience());
			if (ret == 1) break;
		}
		else if(Buffer[0] == 'r' && Buffer[1] == 'u' && Buffer[2] == 'n' && isRun==false)  //如果是第一次使用
		{		
			const char* sendData = "lisence ？";
			ret = send(sock, sendData);
			if (ret == 1) break;
			recv(sock, Buffer);
			if (ret == 1) break;
			if (run(Buffer))
			{
				lience = Buffer;
				sendData = "allow run";
				isRun = true;
			}
			else
				sendData = "do not allow run,please try a minute later";
			ret = send(sock, sendData);
			if (ret == 1) break;
		}
		else if (Buffer[0] == 'e' && Buffer[1] == 'x' && Buffer[2] == 'i' && Buffer[3]=='t')  //如果是退出
		{
			break;
		}
		else if (isRun == false)
		{
			const char* sendData = "lisence ？";
			ret = send(sock, sendData);
			if (ret == 1) break;
		}
		else if (isRun = true)
		{
			const char* sendData = "I get it.";
			ret = send(sock, sendData);
			if (ret == 1) break;
		}
	}
	if (isRun == true)
		exit(lience);
	printf("server>> a client exit.\n");
	return 0;
}



void server()
{
	//开启网络库，初始化WSA 
	WSADATA   wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		printf("WinSock error!\n");
		return;
	}

	//创建服务器Socket(监听套接字)
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == SOCKET_ERROR) {
		printf("create socket() error: %d\n", WSAGetLastError());
		return;
	}

	//绑定IP和端口
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (bind(sListen, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}

	//打开监听
	listen(sListen, MAX);
	printf("wait connect...\n");

	//在端口进行监听，一旦有客户机发起连接请示,就建立与客户机进行通信的线程      
	DWORD        dwThread;
	SOCKET        sClient;
	int            AddrSize;
	struct sockaddr_in client;
	while (1)
	{
		AddrSize = sizeof(client);
		//监听是否有连接请求
		sClient = accept(sListen,(struct sockaddr*)&client,&AddrSize);
		if (sClient == INVALID_SOCKET) {
			printf("accept() error: %d\n", WSAGetLastError());
			break;
		}
		printf("connect: %s:%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));

		//创建一个线程去处理
		HANDLE	hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)sClient, 0, &dwThread);

		if (hThread == NULL) {
			printf("CreateThread() error: %d\n", GetLastError());
			break;
		}
		//处理完后关闭
		CloseHandle(hThread);
	}
	closesocket(sListen);
	WSACleanup();    //用完清理
	return;
}

int main(int argc, char* argv[])
{
	//服务器端代码  
	server();
	return 0;
}
