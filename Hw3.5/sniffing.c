#include<stdio.h>

#include<string.h>
#include <net/ethernet.h>
#include<stdlib.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <linux/ipv6.h>
#include <pcap.h>
#include <pcap/pcap.h>
#include <bits/endian.h>
#include <arpa/inet.h>
#include<netinet/in.h>

#include<time.h>
#include <arpa/inet.h>
#define BUFSIZE 10240

#define STRSIZE 1024

void handle_packet(u_char* args,const struct pcap_pkthdr* header,const u_char* packet){

    printf("---------------------PACKET RECEIVE-----------------------\n");

	/* The total packet length, including all headers
       and the data payload is stored in
       header->len and header->caplen. Caplen is
       the amount actually available, and len is the
       total packet length even if it is larger
       than what we currently have captured. If the snapshot
       length set with pcap_open_live() is too small, you may
       not have the whole packet. */

	//figure out what protocol is using
	
    struct ether_header* eth_header;
    eth_header = (struct ether_header*)packet;

    printf("Total packet available: %d bytes\n", header->caplen);
    printf("Expected packet size: %d bytes\n", header->len);

    //print time
    struct tm *lt;
	char timestr[80];
	time_t local_tv_sec;
	local_tv_sec = header->ts.tv_sec;
	lt = localtime(&local_tv_sec);
	strftime(timestr, sizeof(timestr), "%b %d %Y, %X", lt);
    printf("Receive Time: %s\n",timestr);
    //print MAC address
    printf("Src MAC address : %02X : %02X : %02X : %02X : %02X : %02X\n" , eth_header->ether_shost[0],eth_header->ether_shost[1],eth_header->ether_shost[2],eth_header->ether_shost[3],eth_header->ether_shost[4],eth_header->ether_shost[5]);
    printf("Dst MAC address : %02x : %02x : %02x : %02x : %02x : %02x\n" , eth_header->ether_dhost[0],eth_header->ether_dhost[1],eth_header->ether_dhost[2],eth_header->ether_dhost[3],eth_header->ether_dhost[4],eth_header->ether_dhost[5]);
    printf("Ether Type = 0x%04x\n" , eth_header->ether_type);
    /* Pointers to start point of various headers */
    struct iphdr* ipv4_header;
    struct ipv6hdr* ipv6_header;
    struct tcphdr* tcp_header;
    struct udphdr* udp_header;

    /* Header lengths in bytes */
	if(eth_header->ether_type == ETHERTYPE_IP || ETHERTYPE_IPV6){  //is a IP packet
        ipv4_header = (struct iphdr*)(packet + sizeof(struct ether_header));
        if(ipv4_header->version == 6){  //is a IPV6 packet
            ipv6_header = (struct ipv6hdr*)(packet + sizeof(struct ether_header));
            printf("It's a IPv6 packet.\n");
            printf("Src IP = ");
            for(int i=0;i<16;i+=2){
                printf("%2x%2x:",ipv6_header->saddr.s6_addr[i],ipv6_header->saddr.s6_addr[i+1]);
            }
            printf("\nDest IP =");
            for(int i=0;i<16;i+=2){
                printf("%2x%2x:",ipv6_header->daddr.s6_addr[i],ipv6_header->daddr.s6_addr[i+1]);
            }
        }
        else{
            printf("It's a IPV4 packet.\n");
            printf("Src addr = %s\n", inet_ntoa(*(struct in_addr*) &ipv4_header->saddr) );
            printf("Dst addr = %s\n", inet_ntoa(*(struct in_addr*) &ipv4_header->daddr) );
        }

        if(ipv4_header->protocol == IPPROTO_TCP){ //is a TCP protocol
            tcp_header = (struct tcphdr*)(packet+sizeof(struct ether_header)+sizeof(struct iphdr));
            printf("It's a TCP packet.\n");
            printf("Src Port = %4X\n" , tcp_header->source);
            printf("Dst Port = %4X\n" , tcp_header->dest);
        }
        else if(ipv4_header->protocol == IPPROTO_UDP){ //is a UDP protocol
            udp_header = (struct udphdr*)(packet+sizeof(struct ether_header)+sizeof(struct iphdr));
            printf("It's a UDP packet.\n");
            printf("Src Port = %4X\n" , udp_header->source);
            printf("Dst Port = %4X\n" , udp_header->dest);
        }


    }

    return;
}




int main(int argc,char** argv){
    pcap_t *handle;			/* Session handle */
	char *dev;			/* The device to sniff on */
	char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
    char* file;
	bpf_u_int32 mask = PCAP_NETMASK_UNKNOWN;		/* Our netmask */
	bpf_u_int32 net;		/* Our IP */
	struct pcap_pkthdr header;	/* The header that pcap gives us */
	const u_char *packet;		/* The actual packet */


	if(argc < 2){
		perror("please input file!\n");
		exit(0);
	}
	else{
		file = argv[1];
	}
	handle = pcap_open_offline_with_tstamp_precision(file, PCAP_TSTAMP_PRECISION_MICRO, errbuf);
	if(handle == NULL){
		perror("can not open file : ");
		return 2;
	}

	printf("Analysising...\n");
	pcap_loop(handle,0,handle_packet,NULL);
	/* And close the session */
	pcap_close(handle);
	return(0);
}