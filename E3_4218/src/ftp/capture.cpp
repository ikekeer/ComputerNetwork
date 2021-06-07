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

//mac头部
typedef struct mac_header
{
	u_char dest_addr[6];  //目的地址
	u_char src_addr[6];  //源地址
	u_char type[2];    //协议类型
}mac_header;

//ip头部，20字节
typedef struct ip_header
{
	u_char ver_ihl;   //版本
	u_char tos;      //服务类型
	u_short tlen;    //总长度
	u_short identification;  //标志字段
	u_short flags_fo;   //标记字段
	u_char ttl;   //生存期TTL
	u_char proto;  //上层协议
	u_char crc;    //校验和
	u_char saddr[4];  //源IP地址
	u_char daddr[4];  //目的IP地址
	u_int op_pad;  //选项与填充
}ip_header;

//tcp头部，总长度20字节
typedef struct tcp_header
{
	u_short sport;  //源端口号
	u_short dport;  //目的端口号
	u_int th_seq;   //序列号
	u_int th_ack;  //确认号
	u_int th1 : 4;     //tcp头部长度
	u_int th_res : 4;  //6位中的4位首部长度
	u_int th_res2 : 2;  //6位中的2位首部长度
	u_char th_flags;   //6位标志位
	u_short th_win;   //16位窗口大小
	u_short th_sum;  //16位tcp校验和
	u_short th_urp; //16位紧急指针
}tcp_header;

typedef struct ftp_info
{
	string user;
	string pass;
	string info;
}ftp_info;

//函数声明
void print(const u_char* pkt_data);
string bytearray2hex(unsigned char byte_arr[], int arr_len);
string bytearray2dec(unsigned char byte_arr[], int arr_len);

//ftp、源MAC、源IP、目标MAC、目标IP、帧长度
ftp_info ftp;


void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data)
{
	mac_header* mh;
	tcp_header* uh;
	u_int ip_len;
	u_short sport, dport;
	int head = 54;  //14位以太网头，20位ip头，20位tcp头

	//选择出command为user和pass的包
	string com;//读取
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

//输出数据
void print(const u_char* pkt_data)
{
	mac_header* mh;
	ip_header* ih;
	int length = sizeof(mac_header) + sizeof(ip_header);

	//获取mac、ip
	mh = (mac_header*)pkt_data;
	ih = (ip_header*)(pkt_data + sizeof(mac_header));

	//获取当前时间
	time_t rawTime = time(0);
	tm* info = localtime(&rawTime);
	char nowTime[30];
	strftime(nowTime, 30, "%Y-%m-%d %H:%M:%S", info);

	//获取源MAC、源IP、目标MAC、目标IP、帧长度
	string smaddr = bytearray2hex(mh->src_addr, 6);
	string siaddr = bytearray2dec(ih->saddr, 4);
	string dmaddr = bytearray2hex(mh->dest_addr, 6);
	string diaddr = bytearray2dec(ih->daddr, 4);

	//写入csv文件
	ofstream oFile;
	//打开要输出的文件
	oFile.open("ftpInfo.csv", ios::out | ios::app);
	oFile << nowTime << "," << smaddr << "," << siaddr << ","
		<< dmaddr << "," << diaddr << "," << ftp.user << ","
		<< ftp.pass << "," << ftp.info << endl;
	oFile.close();

	cout << nowTime << "," << smaddr << "," << siaddr << ","
		<< dmaddr << "," << diaddr << "," << ftp.user << ","
		<< ftp.pass << "," << ftp.info << endl;
}

//利用sprintf函数把格式化的ip地址写入字符串
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

//把格式化的mac地址写入字符串
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

	
	/*取得列表*/
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING ,NULL,&alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pacp_findalldevs:%s\n", errbuf);
		exit(1);
	}
	
	/*输出列表*/
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
		//释放列表
		pcap_freealldevs(alldevs);
		return -1;
	}

	//转到选择的设备
	for (d = alldevs,i = 0;i < inum - 1;d = d->next, i++);

	//打开设备
	if ((adhandle = pcap_open_live(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, errbuf)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter.%s is not supported by WinPcap\n");
		pcap_freealldevs(alldevs);   //释放列表
		return -1;
	}

	//预处理
	if (pcap_datalink(adhandle) != DLT_EN10MB)  //检查链路层，只简单支持以太网
	{
		fprintf(stderr, "\nThis program works only on Ethernet networks.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	u_long netmask;
	if (d->addresses != NULL)  //检索接口的第一个地址的掩码
	{
		netmask = ((struct sockaddr_in*)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	}
	else netmask = 0xffffff; //如果接口没有地址，假设在一个C类网络

	//编译过滤器
	bpf_program fcode;
	char packet_filter[] = "tcp";
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) < 0)
	{
		fprintf(stderr, "\nUnable to compile the packet fliter.Check the syntax.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	//设置过滤器
	if (pcap_setfilter(adhandle, &fcode) < 0)
	{
		fprintf(stderr, "\nError setting the fliter.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	printf("\nlistening on %s...\n", d->description);

	//写csv文件头
	ofstream oFile;
	oFile.open("ftpInfo.csv", ios::out | ios::trunc);
	oFile << "时间" << "," << "源MAC地址"<< "," << "源IP地址" << "," 
		<< "目标MAC地址"<< "," <<"目标IP地址"<< "," << "登录名" << "," 
		<< "口令" << "," << "成功与否" << endl;
	oFile.close();

	//释放列表
	pcap_freealldevs(alldevs);
	pcap_loop(adhandle, -1, packet_handler, NULL);
	char c = getchar();
	return 0;
}