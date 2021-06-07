#define HAVE_REMOTE
#define _CRT_SECURE_NO_WARNINGS

#include <pcap.h>
#include <ntddndis.h>
#include <Packet32.h>
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <iostream>
#include <ctime>
using namespace std;

#pragma comment(lib,"Packet")
#pragma comment(lib,"wpcap")
#pragma comment(lib,"WS2_32")

//macͷ��
typedef struct mac_header
{
	u_char dest_addr[6];  //Ŀ�ĵ�ַ
	u_char src_addr[6];  //Դ��ַ
	u_char type[2];    //Э������
}mac_header;

//ipͷ����20�ֽ�
typedef struct ip_header
{
	u_char ver_ihl;   //�汾
	u_char tos;      //��������
	u_short tlen;    //�ܳ���
	u_short identification;  //��־�ֶ�
	u_short flags_fo;   //����ֶ�
	u_char ttl;   //������TTL
	u_char proto;  //�ϲ�Э��
	u_char crc;    //У���
	u_char saddr[4];  //ԴIP��ַ
	u_char daddr[4];  //Ŀ��IP��ַ
	u_int op_pad;  //ѡ�������
}ip_header;

//tcpͷ�����ܳ���20�ֽ�
typedef struct tcp_header
{
	u_short sport;  //Դ�˿ں�
	u_short dport;  //Ŀ�Ķ˿ں�
	u_int th_seq;   //���к�
	u_int th_ack;  //ȷ�Ϻ�
	u_int th1 : 4;     //tcpͷ������
	u_int th_res : 4;  //6λ�е�4λ�ײ�����
	u_int th_res2 : 2;  //6λ�е�2λ�ײ�����
	u_char th_flags;   //6λ��־λ
	u_short th_win;   //16λ���ڴ�С
	u_short th_sum;  //16λtcpУ���
	u_short th_urp; //16λ����ָ��
}tcp_header;

typedef struct ftp_info
{
	string user;
	string pass;
	string info;
}ftp_info;

//��������
void print(const u_char* pkt_data);
string bytearray2hex(unsigned char byte_arr[], int arr_len);
string bytearray2dec(unsigned char byte_arr[], int arr_len);

//ftp��ԴMAC��ԴIP��Ŀ��MAC��Ŀ��IP��֡����
ftp_info ftp;


void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data)
{
	mac_header* mh;
	tcp_header* uh;
	u_int ip_len;
	u_short sport, dport;
	int head = 54;  //14λ��̫��ͷ��20λipͷ��20λtcpͷ

	//ѡ���commandΪuser��pass�İ�
	string com;//��ȡ
	for (int i = 0; i < 4; i++)
		com+= (char)pkt_data[head + i];
	if (com == "USER")
	{
		ostringstream sout;
		for (int i = head + 5; pkt_data[i] != 13; i++)
		{
			sout << pkt_data[i];
		}
		ftp.user = sout.str();
	}
	else if (com == "PASS")
	{
		ostringstream sout;
		for (int i = head + 5; pkt_data[i] != 13; i++)
		{
			sout << pkt_data[i];
		}
		ftp.pass = sout.str();
	}
	else if (com == "230 ")
	{
		ftp.info = "SUCCEED";
		print(pkt_data);
	}
	else if (com == "530 ")
	{
		ftp.info = "FAILD";
		print(pkt_data);
	}
}

//�������
void print(const u_char* pkt_data)
{
	mac_header* mh;
	ip_header* ih;
	int length = sizeof(mac_header) + sizeof(ip_header);

	//��ȡmac��ip
	mh = (mac_header*)pkt_data;
	ih = (ip_header*)(pkt_data + sizeof(mac_header));

	//��ȡ��ǰʱ��
	time_t rawTime = time(0);
	tm* info = localtime(&rawTime);
	char nowTime[30];
	strftime(nowTime, 30, "%Y-%m-%d %H:%M:%S", info);

	//��ȡԴMAC��ԴIP��Ŀ��MAC��Ŀ��IP��֡����
	string smaddr = bytearray2hex(mh->src_addr, 6);
	string siaddr = bytearray2dec(ih->saddr, 4);
	string dmaddr = bytearray2hex(mh->dest_addr, 6);
	string diaddr = bytearray2dec(ih->daddr, 4);

	//д��csv�ļ�
	ofstream oFile;
	//��Ҫ������ļ�
	oFile.open("ftpInfo.csv", ios::out | ios::app);
	oFile << nowTime << "," << smaddr << "," << siaddr << ","
		<< dmaddr << "," << diaddr << "," << ftp.user << ","
		<< ftp.pass << "," << ftp.info << endl;
	oFile.close();

	cout << nowTime << "," << smaddr << "," << siaddr << ","
		<< dmaddr << "," << diaddr << "," << ftp.user << ","
		<< ftp.pass << "," << ftp.info << endl;
}

//����sprintf�����Ѹ�ʽ����ip��ַд���ַ���
string bytearray2dec(unsigned char byte_arr[], int arr_len)
{
	char  buff[20];
	string res;
	for (int i = 0;i < arr_len;i++)
	{
		if (i < arr_len - 1) sprintf(buff, "%d.", byte_arr[i]);
		else sprintf(buff, "%d", byte_arr[i]);
		res.append(buff);
	}
	return res;
}

char hb2hex(unsigned char hb)

{
	hb = hb & 0xF;
	return hb < 10 ? '0' + hb : hb - 10 + 'A';
}

//�Ѹ�ʽ����mac��ַд���ַ���
string bytearray2hex(unsigned char byte_arr[], int arr_len)
{
	string res;
	for (size_t i = 0; i < arr_len; ++i) {
		res.push_back(hb2hex(byte_arr[i] >> 4));
		res.push_back(hb2hex(byte_arr[i]));
		if (i < arr_len - 1) res.push_back('-');
	}
	return res;
}



int main()
{
	pcap_if_t* alldevs;
	pcap_if_t* d;
	int inum;
	pcap_t* adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	int i = 0;

	
	/*ȡ���б�*/
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING ,NULL,&alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pacp_findalldevs:%s\n", errbuf);
		exit(1);
	}
	
	/*����б�*/
	for (d = alldevs;d != NULL;d = d->next)
	{
		pcap_addr_t* a;
		printf("%d.%s", ++i, d->name);
		if (d->description)
		{
			printf("\tDescription:(%s)\n", d->description);
		}
		else {
			printf("\t(No description available)\n");
		}
	}

	if (i == 0)
	{

		printf("\nNo interfaces found!Make sure WinPcap is installed.\n");
		return -1;
	}

	printf("Enter the interface number (1-%d):", i);
	scanf_s("%d", &inum);
	if (inum <1 || inum >i)
	{
		printf("\nInterface number out of range.\n");
		//�ͷ��б�
		pcap_freealldevs(alldevs);
		return -1;
	}

	//ת��ѡ����豸
	for (d = alldevs,i = 0;i < inum - 1;d = d->next, i++);

	//���豸
	if ((adhandle = pcap_open_live(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, errbuf)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter.%s is not supported by WinPcap\n");
		pcap_freealldevs(alldevs);   //�ͷ��б�
		return -1;
	}

	//Ԥ����
	if (pcap_datalink(adhandle) != DLT_EN10MB)  //�����·�㣬ֻ��֧����̫��
	{
		fprintf(stderr, "\nThis program works only on Ethernet networks.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	u_long netmask;
	if (d->addresses != NULL)  //�����ӿڵĵ�һ����ַ������
	{
		netmask = ((struct sockaddr_in*)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	}
	else netmask = 0xffffff; //����ӿ�û�е�ַ��������һ��C������

	//���������
	bpf_program fcode;
	char packet_filter[] = "tcp";
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) < 0)
	{
		fprintf(stderr, "\nUnable to compile the packet fliter.Check the syntax.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	//���ù�����
	if (pcap_setfilter(adhandle, &fcode) < 0)
	{
		fprintf(stderr, "\nError setting the fliter.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	printf("\nlistening on %s...\n", d->description);

	//дcsv�ļ�ͷ
	ofstream oFile;
	oFile.open("ftpInfo.csv", ios::out | ios::trunc);
	oFile << "ʱ��" << "," << "ԴMAC��ַ"<< "," << "ԴIP��ַ" << "," 
		<< "Ŀ��MAC��ַ"<< "," <<"Ŀ��IP��ַ"<< "," << "��¼��" << "," 
		<< "����" << "," << "�ɹ����" << endl;
	oFile.close();

	//�ͷ��б�
	pcap_freealldevs(alldevs);
	pcap_loop(adhandle, -1, packet_handler, NULL);
	char c = getchar();
	return 0;
}