#ifndef __PCAP2VXLAN_H__
#define __PCAP2VXLAN_H__
#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
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

#define DLT_EN10MB 1 /* Ethernet (10Mb) */
#define PCAP_SNAPLEN 0xffff

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

/*
 * VXLAN Header:
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ -+-+-+-+-+-+-+-+-+-+-+
 * | R | R | R | R | I | R | R | R | Reserved |
 * +-+-+-+ -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ -+-+-+-+
 * | VXLAN Network Identifier (VNI) | Reserved |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

struct vxlan
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint32_t r6_8 : 3; /* 6 to 8 R bit */
	uint32_t i : 1; /* I bit */
	uint32_t r1_4 : 4; /* 1 to 4 R bit */
#elif __BYTE_ORDER == __BIG_ENDIAN
	uint32_t r1_4 : 4; /* 1 to 4 R bit */
	uint32_t i : 1; /* I bit */
	uint32_t r6_8 : 3; /* 6 to 8 R bit */
#endif

	uint32_t pad1 : 24;

	uint32_t vni : 24;
	uint32_t pad2 : 8;
	};

typedef struct sys_conf
	{
	struct ether_addr* src_mac;
	struct ether_addr* dst_mac;
	struct in_addr src_ip;
	struct in_addr dst_ip;
	uint32_t vxlan_port;
	uint32_t vxlan_vni;
	} conf_t;

#endif //__PCAP2VXLAN_H__
