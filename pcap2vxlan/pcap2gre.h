#ifndef __PCAP2GRE_H__
#define __PCAP2GRE_H__

#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __STDC_FORMAT_MACROS

#define CONF_INT 0
#define CONF_STR 1
#define CONF_SIZE 1024
#define TCPDUMP_MAGIC 0xa1b2c3d4
#define PCAP_VERSION_MAJOR 2
#define PCAP_VERSION_MINOR 4
#define DLT_EN10MB 1 /* Ethernet (10Mb) */ #define PCAP_SNAPLEN 0xffff

struct pd_timeval
	{
	uint32_t tv_sec;
	uint32_t tv_usec;
	};
struct pd_pkthdr
	{
	struct pd_timeval ts;
	uint32_t caplen;
	uint32_t len;
	};

typedef struct ether_h
	{
	struct ether_addr ether_dhost;
	struct ether_addr ether_shost;
	uint16_t ntag;
	} ether_t;

	union gre_opt_id {
	uint32_t vsid : 24; /* Virtual Subnet ID (VSID) */
	uint32_t flowid : 8; /* Flow ID */
	};

	union gre_opt {
	struct
		{
		uint32_t csum : 16; /* Checksum */
		uint32_t pad1 : 16;
		};

	/* RFC2890 Support */
	uint32_t key; /* Key */
	uint32_t seq; /* Sequence Number */

	/* NVGRE Support */
	struct  (
		uint32_t     vsid : 24 ;     /* Virtual Subnet ID (VSID) */
		uint32_t     flowid : 8 ;    /* Flow ID */
	};
}
;

struct gre
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t pad1 : 4; /* Reserved0 (bits 4-7) */
	uint8_t sflg : 1; /* Sequence Number Present (bit 3) */
	uint8_t kflg : 1; /* Key Present (bit 2) */
	uint8_t pad0 : 1; /* Padding */
	uint8_t cflg : 1; /* Checksum Present (bit 0) */
	uint8_t ver : 3; /* Version Number (bits 5-7) */
	uint8_t pad2 : 5; /* Reserved0 (bits 0-4) */
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
	uint8_t cflg : 1; /* Checksum Present (bit 0) */
	uint8_t pad0 : 1; /* Padding */
	uint8_t kflg : 1; /* Key Present (bit 2) */
	uint8_t sflg : 1; /* Sequence Number Present (bit 3) */
	uint8_t /* Reserved0 (bits 4-7) */ uint8_t pad2 : 5;
	pad1 : 4;
	/* Reserved0 (bits 0-4) */
	uint8_t ver : 3; /* Version Number (bits 5-7) */
#endif

	uint16_t proto; /* Protocol Type (2 octets) */
	union gre_opt opt[3]; /* Optional Fields */
	};

typedef enum gretype
{
	GRE_RFC2784 = 0,
	GRE_RFC2890,
	GRE_NVGRE,
	GRE_MAX
} gretype_e;

struct sys_conf
	{
	uint32_t gre_type;
	struct in_addr src;
	struct in_addr dst;
	bool checksum_bit;
	bool key_bit;
	bool sequence_num_bit;
	uint32_t key;
	uint32_t sequence_num;
	uint32_t vsid : 24;
	uint32_t flowid : 8;
	} conf_t;

#endif //__PCAP2GRE_H__
