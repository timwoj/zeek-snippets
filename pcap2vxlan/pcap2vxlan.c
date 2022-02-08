#include "pcap2vxlan.h"

void print_usage(void)
	{
	printf("Usage: pcap2vxlan [pcap in] [pcap out] \n ");
	return;
	}

bool parse_element(const char* buf, const int type, const char* findKey, void* getvalue)
	{
	printf("%s\n", buf);
	char key[CONF_SIZE], value[CONF_SIZE];

	if ( sscanf(buf, "%s = %s", key, value) != 2 )
		{
		printf("%s %s\n", key, value);
		if ( sscanf(buf, "%s =", key) == 1 )
			{
			value[0] = '\0';
			}
		else
			{
			printf("pcap2gre.conf format error. %s\n", buf);
			return false;
			}
		}

	if ( strcmp(findKey, key) == 0 )
		{
		if ( CONF_INT == type )
			{
			*(uint32_t*)getvalue = atoi(value);
			return true;
			}
		else
			{
			snprintf((char*)getvalue, CONF_SIZE, "%s", value);
			return true;
			}
		}
	return false;
	}

bool parse_conf(struct sys_conf* conf)
	{
	int ret;
	FILE* fp;
	char row[PATH_MAX];
	char bufs[CONF_SIZE];
	uint32_t BUFi;

	fp = fopen("pcap2vxlan.conf", "r");
	if ( NULL == fp )
		{
		fprintf(stderr, "pcap2gre.conf \n ");
		return false;
		}

	memset(conf, 0x00, sizeof(conf_t));

	while ( NULL != fgets(row, sizeof(row), fp) )
		{
		if (row[strlen(row) - 1] == '\n')
			row[strlen(row) - 1] = '\0';
		if (row[0] == '#' || row[0] == '\0')
			continue;

		if ( true == parse_element(row, CONF_STR, "OUTER_SRC_MAC", bufs) )
			{
			conf->src_mac = ether_aton(bufs);
			assert(conf->src_mac);
			continue;
			}

		if ( true == parse_element(row, CONF_STR, "OUTER_DST_MAC", bufs) )
			{
			conf->dst_mac = ether_aton(bufs);
			assert(conf->dst_mac);
			continue;
			}

		if ( true == parse_element(row, CONF_STR, "OUTER_SRC_IP", bufs) )
			{
			assert(inet_pton(AF_INET, bufs, &conf->src_ip));
			continue;
			}

		if ( true == parse_element(row, CONF_STR, "OUTER_DST_IP", bufs) )
			{
			assert(inet_pton(AF_INET, bufs, &conf->dst_ip));
			continue;
			}

		if ( true == parse_element(row, CONF_INT, "VXLAN_PORT", &conf->vxlan_port) )
			{
			continue;
			}

		if ( true == parse_element(row, CONF_INT, "VXLAN_VNI", &conf->vxlan_vni) )
			{
			continue;
			}
		}

	fclose(fp);
	return true;
	}

unsigned short pkt_checksum(unsigned short* data, size_t len, unsigned short add)
	{
	unsigned int L_sum = add;
		union {
		unsigned char L_8bit[2];
		unsigned short L_16bit;
		} L_Tail;

	while ( len > 1 )
		{
		L_sum += ntohs(*data++);
		len -= 2;
		}

	if ( len == 1 )
		{
		L_Tail.L_16bit = 0;
		L_Tail.L_8bit[0] = *(u_int8_t*)data;
		L_sum += ntohs(L_Tail.L_16bit);
		}

	L_sum = (L_sum & 0xffff) + (L_sum >> 16);
	L_sum = (L_sum & 0xffff) + (L_sum >> 16);

	return ~(unsigned short)L_sum;
	}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused)))
	{
	FILE* fp;
	pcap_t* pd;
	char ebuf[PCAP_ERRBUF_SIZE];
	char cap_in[PATH_MAX];
	char cap_out[PATH_MAX];
	uint32_t i = 0;

	struct pcap_file_header pfhdr;
	struct pcap_pkthdr phdr_in;
	struct pd_pkthdr phdr_out;
	const uint8_t* data;
	const unsigned char* pkt;

	struct sys_conf conf;
	char bufsrc[INET_ADDRSTRLEN];
	char bufdst[INET_ADDRSTRLEN];

	if ( argc > 2 )
		{
		strncpy(cap_in, argv[1], PATH_MAX);
		strncpy(cap_out, argv[2], PATH_MAX);
		}
	else
		{
		print_usage();
		exit(EXIT_FAILURE);
		}

	assert(parse_conf(&conf));

	printf("in [%s] out [%s] \n ", argv[1], argv[2]);
	printf("mac_src [%s] mac_dst [%s] \n ", ether_ntoa(conf.src_mac),
	       ether_ntoa(conf.dst_mac));
	printf("ip_src [%s] ip_dst [%s] \n ",
	       inet_ntop(AF_INET, (const void*)&conf.src_ip, bufsrc, INET_ADDRSTRLEN),
	       inet_ntop(AF_INET, (const void*)&conf.dst_ip, bufdst, INET_ADDRSTRLEN));
	printf("Vxlan_port [%d] Vxlan_vni [%d] \n\n ", conf.vxlan_port, conf.vxlan_vni);

	memset(&pfhdr, 0X00, sizeof(struct pcap_file_header));
	pfhdr.magic = TCPDUMP_MAGIC;
	pfhdr.version_major = PCAP_VERSION_MAJOR;
	pfhdr.version_minor = PCAP_VERSION_MINOR;
	pfhdr.snaplen = PCAP_SNAPLEN;
	pfhdr.linktype = DLT_EN10MB;

	/* ether */
	struct ether_h ether_out;
	struct ether_h ether_in;
	memset(&ether_out, 0x00, sizeof(struct ether_h));
	memset(&ether_in, 0x00, sizeof(struct ether_h));

	memcpy(&ether_out.ether_shost, &conf.src_mac, sizeof(struct ether_addr));
	memcpy(&ether_out.ether_dhost, &conf.dst_mac, sizeof(struct ether_addr));
	ether_out.ntag = 0X0008;

	memcpy(&ether_in.ether_shost, &conf.dst_mac, sizeof(struct ether_addr));
	memcpy(&ether_in.ether_dhost, &conf.src_mac, sizeof(struct ether_addr));
	ether_in.ntag = 0X0008;

	/* ipv4 */
	struct ip iph_out;
	struct ip iph_in;
	memset(&iph_out, 0x00, sizeof(struct ip));
	memset(&iph_in, 0x00, sizeof(struct ip));

	iph_out.ip_v = 4;
	iph_out.ip_hl = 5;
	iph_out.ip_hl = 5;
	iph_out.ip_off = 0x0040;
	iph_out.ip_ttl = 0x40;
	iph_out.ip_p = 0x11;
	memcpy(&iph_out.ip_src, &conf.src_ip, sizeof(struct in_addr));
	memcpy(&iph_out.ip_dst, &conf.dst_ip, sizeof(struct in_addr));

	iph_in.ip_v = 4;
	iph_in.ip_hl = 5;
	iph_in.ip_off = 0x0040;
	iph_in.ip_ttl = 0x40;
	iph_in.ip_p = 0x11;
	memcpy(&iph_in.ip_src, &conf.dst_ip, sizeof(struct in_addr));
	memcpy(&iph_in.ip_dst, &conf.src_ip, sizeof(struct in_addr));

	/* udp */
	struct udphdr udph_out;
	struct udphdr udph_in;
	memset(&udph_out, 0x00, sizeof(struct udphdr));
	memset(&udph_in, 0x00, sizeof(struct udphdr));

	uint16_t dst_port;
	dst_port = conf.vxlan_port;

	udph_out.uh_dport = htons(dst_port);
	udph_in.uh_dport = htons(dst_port);

	/* vxlan */
	struct vxlan vxlanh;
	memset(&vxlanh, 0x00, sizeof(struct vxlan));

	vxlanh.i = 1;

	uint32_t Vni;
	Vni = htonl(conf.vxlan_vni);
	vxlanh.vni = Vni >> 8;

	/* pcap open */
	assert(pd = pcap_open_offline(cap_in, ebuf));
	assert(fp = fopen(cap_out, "wb"));

	fwrite(&pfhdr, sizeof(struct pcap_file_header), 1, fp);

	struct ether_h* eth_hdr;
	struct ether_addr smac;
	uint16_t ip_size;
	uint16_t udp_size;
	uint16_t csum;

	while ( 1 )
		{

		if ( (data = pcap_next(pd, &phdr_in)) != NULL ) {
			eth_hdr = (struct ether_h*)data;

			if  ( ! i )  {
				memcpy(&smac, &eth_hdr->ether_shost, sizeof(struct ether_addr));
				}

			i ++ ;

			printf ( "[%d] caplen [%d] len [%d] ts [%ld.%d] src [%s] dst [%s] \n " ,
			         i, phdr_in.caplen, phdr_in.len, phdr_in.ts.tv_sec, phdr_in.ts.tv_usec ,
			         ether_ntoa ( & eth_hdr -> ether_shost ),  ether_ntoa ( & eth_hdr -> ether_dhost ));

			memset(&phdr_out, 0x00, sizeof(struct pd_pkthdr));
			phdr_out.ts.tv_sec  = (uint32_t) phdr_in.ts.tv_sec ;
			phdr_out.ts.tv_usec = (uint32_t) phdr_in.ts.tv_usec ;
			phdr_out.caplen  = phdr_in.caplen + 50 ;
			phdr_out.len     = phdr_in.len + 50 ;

			fwrite(&phdr_out, sizeof(struct pd_pkthdr), 1, fp );

			ip_size  = 36 + phdr_in.caplen ;
			udp_size = 16 + phdr_in.caplen ;

			/* in or out judgment */
			if ( 0  ==  memcmp ( & smac, & eth_hdr->ether_shost, sizeof(struct ether_addr)))  {
				udph_out.uh_sport = pkt_checksum((unsigned short*)&eth_hdr, sizeof(struct ether_h), 0);
				iph_out.ip_len = htons(ip_size);
				udph_out.uh_ulen = htons(udp_size);
				iph_out.ip_sum = 0;
				csum = pkt_checksum((unsigned short*)&iph_out, 20, 0);
				iph_out.ip_sum = htons(csum);

				fwrite(&ether_out, 14, 1, fp);
				fwrite(&iph_out, 20, 1, fp);
				fwrite(&udph_out, 8, 1, fp);
				fwrite(&vxlanh, 8, 1, fp);
				}
			else
				{
				udph_in.uh_sport = pkt_checksum((unsigned short*)&eth_hdr, sizeof(struct ether_h), 0);
				iph_in.ip_len = htons(ip_size);
				udph_in.uh_ulen = htons(udp_size);
				iph_in.ip_sum = 0;
				csum = pkt_checksum((unsigned short*)&iph_in, 20, 0);
				iph_in.ip_sum = htons(csum);

				fwrite(&ether_in, 14, 1, fp);
				fwrite(&iph_in, 20, 1, fp);
				fwrite(&udph_in, 8, 1, fp);
				fwrite(&vxlanh, 8, 1, fp);
				}

			fwrite(data, phdr_in.caplen, 1, fp);
		}
	else { break; }
	}

pcap_close(pd);
fclose(fp);

exit(EXIT_SUCCESS);
}
