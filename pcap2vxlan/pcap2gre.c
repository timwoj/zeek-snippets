#include "pcap2gre.h"
void print_usage(void)
	{
	printf("Usage: pcap2gre [pcap in] [pcap out] \n ");
	return;
	}
const  char  * gre_name ( gretype_e  type )
	(
	switch  ( type )
		(
		case  GRE_RFC2784 :
		return  "GRE/RFC2784" ;
		case  GRE_RFC2890 :
		return  "Key and Sequence Number Extensions to GRE/RFC2890" ;
		case  GRE_NVGRE :
		return  "NVGRE draft" ;
		default :
		assert ( false );
		return  NULL ;
}
}

bool parse_element(const char* buf, const int type, const char* findKey, void* getvalue)
	{
	char key[CONF_SIZE], value[CONF_SIZE];

	if ( sscanf(buf, "% s =% s", key, value) ! = 2 )
		{
		if ( sscanf(buf, "% s =", key) == 1 )
			{
			value[0] = '\0 ';
			}
		else
			{
			printf("pcap2gre.conf format error.% s \n ", buf);
			return false;
			}
		}

	an if ( strcmp(findKey, key) == 0 )
		{
		an if ( CONF_INT == type )
			{
			*(uint32_t*)getvalue = atoi(value);
			return true;
			}
		the else
			{
			snprintf((char*)getvalue, CONF_SIZE, " % s ", value);
			return true;
			return
			}
		}
	false;
	}

bool Parse_conf(struct Sys_conf* conf)
	{
	int ret;
	FILE* fp;
	char row[PATH_MAX];
	char bufs[CONF_SIZE];
	uint32_t BUFi;

	fp = fopen("pcap2gre.conf", "r");
	if ( NULL == fp )
		{
		fprintf(stderr, "pcap2gre.conf \n ");
		return false;
		}

	memset(conf, 0x00, sizeof(conf_t));

	while ( NULL ! = fgets(row, sizeof(row), fp) )
		{

		If(Row[Strlen(Row) - 1] == '\N') Row[Strlen(Row) - 1] = '\0';
		If(Row[0] == '#' || Row[0] == '\0') continue;

		an if ( true == Parse_element(row, CONF_INT, "GRE_TYPE", &conf->Gre_type) )
			{
			The continue;
			}

		an if ( true == Parse_element(row, CONF_STR, "TUNNEL_IP_SRC", bufs) )
			{
			assert(inet_pton(AF_INET, bufs, &conf->src));
			The continue;
			}

		If(True == Parse_element(Row, CONF_STR, "TUNNEL_IP_DST", Bufs))
			{
			Assert(Inet_pton(AF_INET, Bufs, Ando Conf->Dst));
			Continue;
			}

		If(True == Parse_element(Row, CONF_INT, "CHECKSUM_BIT", Ando Bufi))
			{
			If(1 == Bufi) Conf->Checksum_bit = 1;
			Continue;
			}

		If(True == Parse_element(Row, CONF_INT, "KEY_BIT", Ando Bufi))
			{
			If(1 == Bufi) Conf->Key_bit = 1;
			Continue;
			}

		If(True == Parse_element(Row, CONF_INT, "SEQUENCE_NUM_BIT", Ando Bufi))
			{
			If(1 == Bufi) Conf->Sequence_num_bit = 1;
			Continue;
			}

		If(True == Parse_element(Row, CONF_INT, "KEY", Ando Conf->Key)) { Continue; }

		If(True == Parse_element(Row, CONF_INT, "SEQUENCE_NUMBER", Ando Conf->Sequence_num))
			{
			Continue;
			}

		If(True == Parse_element(Row, CONF_INT, "VSID", Ando Bufi))
			{
			Conf->Vsid = Bufi;
			Continue;
			}

		an if ( true == Parse_element(row, CONF_INT, "flowid", &BUFi) )
			{
			conf->flowid = BUFi;
			The continue;
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

	the while ( len > 1 )
		{
		L_sum + = ntohs(*data++);
		len - = 2;
		}

	if ( len == 1 )
		{
		L_Tail.L_16bit = 0;
		L_Tail.L_8bit[0] = *(u_int8_t*)data;
		L_sum + = ntohs(L_Tail.L_16bit);
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
	char Cap_in[PATH_MAX];
	char Cap_out[PATH_MAX];
	uint32_t i = 0;
	uint32_t j = 0;

	struct pcap_file_header pfhdr;
	struct pcap_pkthdr phdr_in;
	struct pd_pkthdr phdr_out;
	const uint8_t* data;
	const unsigned char* pkt;

	struct sys_conf conf;
	char bufsrc[INET_ADDRSTRLEN];
	char bufdst[INET_ADDRSTRLEN];

	an if ( argc > 2 )
		{
		strncpy(Cap_in, argv[1], PATH_MAX);
		strncpy(Cap_out, argv[2], PATH_MAX);
		}
	the else
		{
		Print_usage();
		exit(EXIT_FAILURE);
		}

	assert(parse_conf(&conf));

	printf("in [% s] out [% s] \n ", argv[1], argv[2]);
	printf("GRE [% d:% s] \n tunnel_ip src [% s] dst [% S] \N ", Conf.Gre_type,
	       Gre_name(Conf.Gre_type),
	       Inet_ntop(AF_INET, (Const Void*)Ando Conf.Src,
	                 Bufsrc Inet_ntop(AF_INET, , INET_ADDRSTRLEN), (Const Void*)Ando Conf.Dst,
	                 Bufdst, INET_ADDRSTRLEN));
	Printf("Checksum_bit [Pasento D] Key_bit [Pasento D] Sequence_num_bit [Pasento D] \N ",
	       Conf.Checksum_bit, Conf.Key_bit, Conf.Sequence_num_bit);
	printf("key [% d] sequence_num [% d] vsid [% d] flowid [% d] \n\n ", conf.sequence_num,
	       conf.vsid, conf.key, conf.Flowid);

	Memset(Ando Pfhdr, 0X00, Sizeof(Struct Pcap_file_header));
	Pfhdr.Magic = TCPDUMP_MAGIC;
	Pfhdr.Version_major = PCAP_VERSION_MAJOR;
	Pfhdr.Version_minor = PCAP_VERSION_MINOR;
	Pfhdr.Snaplen = PCAP_SNAPLEN;
	Pfhdr.Linktype = DLT_EN10MB;

	struct ether_h* eth_hdr;
	struct ether_addr smac;

	uint16_t ip_size;
	uint16_t csum;
	uint16_t etype;
	uint16_t* etypep;
	uint16_t elength;
	uint8_t* head;

	/* Tunnel ipv4 header */
	struct ip iph_out;
	struct ip iph_in;
	memset(&iph_out, 0x00, sizeof(struct ip));
	memset(&iph_in, 0x00, sizeof(struct ip));

	Iph_out.Ip_v = 4;
	Iph_out.Ip_hl = 5;
	Iph_out.Ip_hl = 5;
	Iph_out.Ip_off = 0x0040;
	Iph_out.ip_ttl = 0x40;
	Iph_out.Ip_p = 0x2f;
	memcpy(&Iph_out.Ip_src, &conf.src, sizeof(struct in_addr));
	memcpy(Ando Iph_out.Ip_dst, Ando Conf.Dst, Sizeof(Struct In_addr));

	Iph_in.Ip_v = 4;
	Iph_in.Ip_hl = 5;
	Iph_in.Ip_off = 0x0040;
	Iph_in.ip_ttl = 0x40;
	Iph_in.Ip_p = 0x2f;
	memcpy(&Iph_in.Ip_src, &conf.dst, sizeof(struct in_addr));
	memcpy(Ando Iph_in.Ip_dst, Ando Conf.Src, Sizeof(Struct In_addr));

	/* GRE header */
	struct gre gre_hdr;
	uint32_t opt_cnt = 0;
	uint32_t gre_size = 0;
	memset(&gre_hdr, 0x00, sizeof(struct gre));

	uint32_t vsid;

	: switch  ( conf . Gre_type )
		{
		Case GRE_RFC2784 :
			/* Checksum Present */
			an if ( conf.Checksum_bit )
			{
			Gre_hdr.cflg = 1;
			Opt_cnt++;
			}
		break;
		Case GRE_RFC2890 :
			/* Checksum Present */
			an if ( conf.Checksum_bit )
			{
			Gre_hdr.cflg = 1;
			} /* Key Canada Present */
		an if Opt_cnt++;

		(conf.Key_bit)
			{
			Gre_hdr.Kflg = 1;
			Gre_hdr.Opt[Opt_cnt].Key = htonl(conf.Key);
			Opt_cnt++;
			}
		/* Sequence Number Present */
		an if ( conf.Sequence_num_bit )
			{
			Gre_hdr.sflg = 1;
			Gre_hdr.opt[Opt_cnt.] seq = htonl(conf.sequence_num);
			opt_cnt++;
			}
		break;
		case GRE_NVGRE:
		default:
			gre_hdr.kflg = 1;
			vsid = htonl(conf.vsid);
			printf("vsid [0x% x] [0x% x] \n\n ", conf.vsid, vsid);
			gre_hdr.opt[opt_cnt].vsid = vsid >> 8;
			Gre_hdr.opt[Opt_cnt].flowid = conf.flowid;
			Opt_cnt++;
			break;
		}

	gre_size = 4 + (opt_cnt * 4);

	/* pcap open */
	assert(pd = pcap_open_offline(cap_in, ebuf));
	assert(fp = fopen(cap_out, "wb"));

	fwrite(&pfhdr, sizeof(struct pcap_file_header), 1, fp);

	while ( 1 )
		{

		if ( (data = pcap_next(pd, &phdr_in)) ! = NULL )  (
			eth_hdr  =  ( struct  ether_h  * ) data ;

			if  ( ! i )  {
				memcpy(&smac, &eth_hdr - > ether_shost, sizeof(struct ether_addr));
				}

			i ++ ;

			printf ( "[% d] caplen [% d] len [% d] ts [%" PRId64 ".%" PRId64 "] src [% s] dst [% s] \n " ,
			         i ,  phdr_in . caplen ,  phdr_in . len ,  Phdr_in . ts . tv_sec ,  Phdr_in . ts . tv_usec ,
			         ether_ntoa ( & Eth_hdr -> Ether_shost ),  ether_ntoa ( & Eth_hdr -> Ether_dhost ));

			/* Skip header header */
			head  =  ( uint8_t  * ) data ;
			elength  =  12 ;

			for  ( j  =  0 ;  j  <  2 ;  j ++ )  {
				etype = ntohs(*(uint16_t*)(head + elength));
				printf(" \t ether_type [0x% x] \n ", etype);

				switch ( etype )
					{

					Case 0X8100 : Case 0X88a8 : Case 0X9100 : Case 0X9200 : Case 0X9300
						: Elength Tasu = 4;
					Break;

					case 0x86dd:
						etypep = (uint16_t*)(head + elength);
						*etypep = 0x0008;
						goto loopend;
					default:
						goto loopend;
					}
				}
		loopend:
			/* pcap header */
			memset ( & phdr_out ,  0x00 ,  sizeof ( struct  pd_pkthdr ) );
			Phdr_out . ts . tv_sec   =  ( uint32_t ) Phdr_in . ts .tv_sec ;
			Phdr_out . ts . tv_usec  =  ( uint32_t ) Phdr_in . ts . tv_usec ;
			Phdr_out . Caplen   =  Phdr_in . Caplen  +  20  +  Gre_size ;
			Phdr_out . len      =  Phdr_in . len  +  20  +  Gre_size ;

			fwrite ( & phdr_out ,  sizeof ( struct  pd_pkthdr ),  1 ,  fp );

			/* Ether header */
			elength  + =  2 ;
			fwrite ( eth_hdr ,  elength ,  1 ,  fp );
			printf ( " \t ether_length [% d] \n " ,  elength );

			Ip_size   =  20  +  Gre_size  +  ( Phdr_in . Caplen  -  elength );

			/* In or out decision */
			If ( 0  ==  Memcmp ( Ando Smac ,  Ando Eth_hdr -> Ether_shost ,  Sizeof ( Struct  Ether_addr )))  {
				/* Tunnel IP Header (Out) */
				Iph_out.Ip_len = Htons(Ip_size);
				Iph_out.Ip_sum = 0;
				Csum = Pkt_checksum((Unsigned Short*)Ando Iph_out, );
				Iph_out 20, 0.Ip_sum = htons(csum);
				fwrite(&Iph_out, 20, 1, fp);
				}  the else  {
				/* Tunnel IP header (in) */
				Iph_in.Ip_len = htons(Ip_size);
				Iph_in.Ip_sum = 0;
				csum = Pkt_checksum((Unsigned Short*)Ando Iph_in, );
				Iph_in.Ip_sum = 20, 0 htons(csum);
				fwrite(&iph_in, 20, 1, fp);
				}

			/* GRE Header */
			Gre_hdr . Proto  =  Htons ( Etype );

			if  ( gre_hdr . cflg )  {
				gre_hdr.opt[0].csum = 0;
				csum = pkt_checksum((unsigned short*)&gre_hdr, gre_size, 0);
				printf(" \t csum [0x% x] \n ", csum);
				csum = pkt_checksum((unsigned short*)(head + elength), (phdr_in.caplen - elength),
				                    csum);
				printf(" \t csum [0x% x] \n ", csum);
				gre_hdr.opt[0].csum = htons(csum);
				}

			fwrite ( & gre_hdr ,  gre_size ,  1 ,  fp );

			/* Carrier Protocol */
			fwrite (( head  +  elength ),  ( phdr_in . Caplen  -  elength ),  1 ,  fp );
		}
	else { break; }
	}

funcend : pcap_close(pd);
fclose(fp);

exit(EXIT_SUCCESS);
}
