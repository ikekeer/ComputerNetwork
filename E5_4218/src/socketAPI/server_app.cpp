#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define DEFAULT_BUFFER 4096 //��������С
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
	long lience = 0; //���к�
	int sum = 0;  //���к�����ʹ������
	int count = 0; //��ǰʹ������
}Lience;

list<Lience> lience_list;

//������������Ϣ
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
	printf("client>> %s\n", Buffer);    //��ӡ���յ�����Ϣ
	return 0;
}

//������������Ϣ
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

//�������֤
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

//���У����֤ʹ��������һ
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

//�˳���ʹ���֤ʹ��������һ
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

//��ͻ���ͨ�ŵ��̺߳���
DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET sock = (SOCKET)lpParam;
	char Buffer[DEFAULT_BUFFER];
	char* lience=NULL;
	int ret;
	bool isRun = false;
	
	while (1) 
	{
		//�������Կͻ�������Ϣ
		ret=recv(sock,Buffer);
		if (ret == 1) break;

		//����ָ��������Ӧ
		if (Buffer[0]=='b' && Buffer[1]=='u' && Buffer[2]=='y')  //����ǹ������֤����
		{
			const char* sendData = "user ��";
			ret=send(sock, sendData);
			if (ret == 1) break;
			recv(sock, Buffer);
			if (ret == 1) break;
			ret = send(sock, generate_lience());
			if (ret == 1) break;
		}
		else if(Buffer[0] == 'r' && Buffer[1] == 'u' && Buffer[2] == 'n' && isRun==false)  //����ǵ�һ��ʹ��
		{		
			const char* sendData = "lisence ��";
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
		else if (Buffer[0] == 'e' && Buffer[1] == 'x' && Buffer[2] == 'i' && Buffer[3]=='t')  //������˳�
		{
			break;
		}
		else if (isRun == false)
		{
			const char* sendData = "lisence ��";
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
	//��������⣬��ʼ��WSA 
	WSADATA   wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		printf("WinSock error!\n");
		return;
	}

	//����������Socket(�����׽���)
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == SOCKET_ERROR) {
		printf("create socket() error: %d\n", WSAGetLastError());
		return;
	}

	//��IP�Ͷ˿�
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (bind(sListen, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}

	//�򿪼���
	listen(sListen, MAX);
	printf("wait connect...\n");

	//�ڶ˿ڽ��м�����һ���пͻ�������������ʾ,�ͽ�����ͻ�������ͨ�ŵ��߳�      
	DWORD        dwThread;
	SOCKET        sClient;
	int            AddrSize;
	struct sockaddr_in client;
	while (1)
	{
		AddrSize = sizeof(client);
		//�����Ƿ�����������
		sClient = accept(sListen,(struct sockaddr*)&client,&AddrSize);
		if (sClient == INVALID_SOCKET) {
			printf("accept() error: %d\n", WSAGetLastError());
			break;
		}
		printf("connect: %s:%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));

		//����һ���߳�ȥ����
		HANDLE	hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)sClient, 0, &dwThread);

		if (hThread == NULL) {
			printf("CreateThread() error: %d\n", GetLastError());
			break;
		}
		//�������ر�
		CloseHandle(hThread);
	}
	closesocket(sListen);
	WSACleanup();    //��������
	return;
}

int main(int argc, char* argv[])
{
	//�������˴���  
	server();
	return 0;
}
