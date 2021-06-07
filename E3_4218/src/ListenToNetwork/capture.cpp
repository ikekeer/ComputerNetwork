#define HAVE_REMOTE
#define _CRT_SECURE_NO_WARNINGS

#include <pcap.h>
#include <ntddndis.h>
#include <Packet32.h>
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <ctime>
using namespace std;

#pragma comment(lib,"Packet")
#pragma comment(lib,"wpcap")
#pragma comment(lib,"WS2_32")

string bytearray2hex(unsigned char byte_arr[], int arr_len);
string bytearray2dec(unsigned char byte_arr[], int arr_len);

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

typedef struct mac_header
{
	u_char dest_addr[6];  //目的地址
	u_char src_addr[6];  //源地址
	u_char type[2];    //协议类型
}mac_header;


void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data)
{
	mac_header* mh;
	ip_header* ih;
	int length = sizeof(mac_header) + sizeof(ip_header);
	for (int i = 0;i < length;i++)
	{
		printf("%02X ", pkt_data[i]); //按二进制输出数据
		if ((i & 0xf) == 0xf) printf("\n");
	}
	printf("\n");

	//捕获后的处理
	mh = (mac_header*)pkt_data;
	printf("mac_header:\n");
	printf("\tdest_addr: ");
	for (int i = 0;i < 6;i++)
	{
		printf("%02X ", mh->dest_addr[i]);
	}
	printf("\n");
	printf("\tsrc_addr : ");
	for (int i = 0;i < 6;i++)
	{
		printf("%02X ", mh->src_addr[i]);
	}
	printf("\n");
	printf("\ttype: %04X", ntohs((u_short)mh->type));
	printf("\n");

	ih = (ip_header*)(pkt_data + sizeof(mac_header));
	printf("ip_header\n");
	printf("\t%-10s: %02X\n", "ver_ihl", ih->ver_ihl);
	printf("\t%-10s: %02X\n", "tos", ih->tos);
	printf("\t%-10s: %04X\n", "tlen", ntohs(ih->tlen));
	printf("\t%-10s: %04X\n", "identification", ntohs(ih->flags_fo));
	printf("\t%-10s: %04X\n", "flags_fo", ntohs(ih->flags_fo));
	printf("\t%-10s: %02X\n", "ttl", ih->ttl);
	printf("\t%-10s: %02X\n", "proto", ih->proto);
	printf("\t%-10s: %04X\n", "crc", ntohs(ih->crc));
	printf("\t%-10s: %08X\n", "op_pad", ntohs(ih->op_pad));
	
	printf("\t%-10s: ", "saddr");
	for (int i = 0;i < 4;i++)
	{
		printf("%02X ", ih->saddr[i]);
	}
	printf(" ");
	for (int i = 0;i < 4;i++)
	{
		printf("%d.", ih->saddr[i]);
	}
	printf("\n");

	printf("\t%-10s: ", "daddr");
	for (int i = 0;i < 4;i++)
	{
		printf("%02X ", ih->daddr[i]);
	}
	printf(" ");
	for (int i = 0;i < 4;i++)
	{
		printf("%d.", ih->daddr[i]);
	}
	printf("\n\n");

	//获取当前时间
	time_t rawTime = time(0);
	tm* info = localtime(&rawTime);
	char nowTime[30];
	strftime(nowTime, 30, "%Y-%m-%d %H:%M:%S", info);

	//获取源MAC、源IP、目标MAC、目标IP、帧长度
	string smaddr=bytearray2hex( mh->src_addr ,6);
	string siaddr = bytearray2dec(ih->saddr, 4);
	string dmaddr = bytearray2hex(mh->dest_addr, 6);
	string diaddr = bytearray2dec(ih->daddr, 4);

	//写入csv文件
	ofstream oFile;
	//打开要输出的文件
	oFile.open("ListenData.csv", ios::out | ios::app);
	oFile << nowTime << "," << smaddr << "," << siaddr << "," << dmaddr << ","<< diaddr<<","<< header->caplen<<endl;
	oFile.close();

}


//利用sprintf函数把格式化的ip地址写入字符串
string bytearray2dec(unsigned char byte_arr[], int arr_len)
{
	char  buff[20];
	string res;
	for (int i = 0;i < arr_len;i++)
	{
		if(i<arr_len-1) sprintf(buff, "%d.", byte_arr[i]);
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
	char packet_filter[] = "ip and udp";
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
	oFile.open("ListenData.csv", ios::out | ios::trunc);
	oFile << "时间" << "," << "源MAC地址"<< "," << "源IP地址" << "," << "目标MAC地址"<< "," <<"目标IP地址"<< "," << "帧长度" << endl;
	oFile.close();

	//释放列表
	pcap_freealldevs(alldevs);
	//开始捕捉
	while (1)
	{
		pcap_loop(adhandle, 1, packet_handler, NULL);
		Sleep(10000); //每10秒捕获一个文件
	}	
	char c = getchar();
	return 0;
}