#include<stdio.h>

#include<string.h>

#include<stdlib.h>

#include <pcap.h>
#include <pcap/pcap.h>

#include<netinet/in.h>

#include<time.h>
#include <arpa/inet.h>
#define BUFSIZE 10240

#define STRSIZE 1024

void handle_packet(u_char* args,const struct pcap_pkthdr* header,const u_char* packet){
	//figure out what protocol is using
	struct ether_header* eth_header;
	eth_header = (struct ether_header*) (packet);

    printf("---------------------PACKET RECEIVE-----------------------\n");

	/* The total packet length, including all headers
       and the data payload is stored in
       header->len and header->caplen. Caplen is
       the amount actually available, and len is the
       total packet length even if it is larger
       than what we currently have captured. If the snapshot
       length set with pcap_open_live() is too small, you may
       not have the whole packet. */
    printf("Total packet available: %d bytes\n", header->caplen);
    printf("Expected packet size: %d bytes\n", header->len);


    struct tm *lt;
	char timestr[80];
	time_t local_tv_sec;

	int size_ip;
	int size_tcp;
	int size_udp;
	int size_payload;

	local_tv_sec = header->ts.tv_sec;
	lt = localtime(&local_tv_sec);
	strftime(timestr, sizeof(timestr), "%b %d %Y, %X", lt);
    printf("Receive Time: %s\n",timestr);


    /* Pointers to start point of various headers */
	const u_char *ip_header;
    const u_char *tcp_header;
    const u_char *payload;

    /* Header lengths in bytes */
    int ethernet_header_length = 14; /* Doesn't change */
    int ip_header_length;
    int tcp_header_length;
    int payload_length;

	ip_header = packet;
	/* Get MAC Address */
	printf("Dest Mac Address :");
	for(int i=0;i<4;i++,ip_header++){
		printf("%02x ",*ip_header);
	}
	printf("\nSrc Mac Address : ");
	for(int i=0;i<4;i++,ip_header++){
		printf("%02x ",*ip_header);
	}
    printf("\nEthernet Type : 0x");
    for(int i=0;i<2;i++,ip_header++){
        printf("%02x",*ip_header);
    }


    /* Find start of IP header */
    ip_header = packet + ethernet_header_length;
    /* The second-half of the first byte in ip_header
       contains the IP header length (IHL). */
    ip_header_length = ((*ip_header) & 0x0F);
    /* The IHL is number of 32-bit segments. Multiply
       by four to get a byte count for pointer arithmetic */
    ip_header_length = ip_header_length * 4;
	//process IP header
	if(ip_header_length<20){
		printf("Invalid IP header!\n");
		return;
	}
	else{
		printf("\nIP header length (IHL) in bytes: %d", ip_header_length);
        ip_header = ip_header + 12;
        printf("\nSrc IP address : 0x");
        for(int i=0;i<4;i++,ip_header++){
            printf("%02x",*ip_header);
        }
        printf("\nDest IP address : 0x");
        for(int i=0;i<4;i++,ip_header++){
		    printf("%02x",*ip_header);
	    }
	}
    

    /* Now that we know where the IP header is, we can 
       inspect the IP header for a protocol number to 
       make sure it is TCP before going any further. 
       Protocol is always the 10th byte of the IP header */
    ip_header = packet + ethernet_header_length;
    u_char protocol = *(ip_header + 9);  //+1 -> +1byte
    printf("\nProtocol Type(in hex) : %02x",protocol);
    if (protocol != IPPROTO_TCP && protocol != IPPROTO_UDP) {
        printf("\nNot a TCP/UDP packet.\n\n");
        return;
    }
    else if(protocol == IPPROTO_TCP){
        /* process TCP package */
        tcp_header = packet + ethernet_header_length + ip_header_length;
        printf("\nIs TCP , src port = 0x%02x" , *tcp_header++);
        printf(" %02x",*tcp_header++);
        printf("\nDes port = %02x" , *tcp_header++);
        printf(" %02x",*tcp_header++);
    }
    else{
        printf("\nIs UDP , src port = 0x%02x" , *tcp_header++);
        printf("%02x",*tcp_header++);
        printf("\nDes port = 0x%02x" , *tcp_header++);
        printf("%02x",*tcp_header++);
    }

    /* Add the ethernet and ip header length to the start of the packet
       to find the beginning of the TCP header */
    tcp_header = packet + ethernet_header_length + ip_header_length;
    /* TCP header length is stored in the first half 
       of the 12th byte in the TCP header. Because we only want
       the value of the top half of the byte, we have to shift it
       down to the bottom half otherwise it is using the most 
       significant bits instead of the least significant bits */
    tcp_header_length = ((*(tcp_header + 12)) & 0xF0) >> 4;
    
    
    /* The TCP header length stored in those 4 bits represents
       how many 32-bit words there are in the header, just like
       the IP header length. We multiply by four again to get a
       byte count. */
    //tcp_header_length = tcp_header_length * 4;
    //printf("\nTCP header length in bytes: %d\n", tcp_header_length);



    /* Add up all the header sizes to find the payload offset */
    int total_headers_size = ethernet_header_length+ip_header_length+tcp_header_length;
    printf("\nSize of all headers combined: %d bytes\n", total_headers_size);
    payload_length = header->caplen -
        (ethernet_header_length + ip_header_length + tcp_header_length);
    printf("Payload size: %d bytes\n", payload_length);
    payload = packet + total_headers_size;
    printf("Memory address where payload begins: %p\n\n", payload);


    /* Print payload in ASCII */
    /*  
    if (payload_length > 0) {
        const u_char *temp_pointer = payload;
        int byte_count = 0;
        while (byte_count++ < payload_length) {
            printf("%c", *temp_pointer);
            temp_pointer++;
        }
        printf("\n");
    }
    */

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