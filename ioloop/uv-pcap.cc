#include <stdio.h>
#include <unistd.h>
#include <uv.h>
#include <pcap.h>

pcap_t* pd;
uv_poll_t poller;

void pcap_hdlr(u_char* user, const struct pcap_pkthdr* h, const u_char* bytes)
	{
	printf("got packet of length %d\n", h->len);
	}

void pcap_callback(uv_poll_t* handle, int status, int events)
	{
	struct pcap_pkthdr hdr;
	int res = pcap_dispatch(pd, 1, pcap_hdlr, NULL);
	printf("(status = %d, events = %d) dispatch returned %d\n", status, events, res);
	if ( res == 0 )
		{
		printf("out of data\n");
		uv_poll_stop(&poller);
		}
	}

bool init_pcap(char* path)
	{
	char errbuf[PCAP_ERRBUF_SIZE];
	pd = pcap_open_offline(path, errbuf);

	if ( ! pd )
		{
		printf("failed to open pcap: %s\n", errbuf);
		return false;
		}

	if ( uv_poll_init(uv_default_loop(), &poller, fileno(pcap_file(pd))) == 0 )
		{
		printf("calling uv_poll_start for pcap\n");
		uv_poll_start(&poller, UV_READABLE, pcap_callback);
		}

	return true;
	}

int main(int argc, char** argv)
	{
	if ( init_pcap(argv[1]) )
		{
		printf("calling uv_run for pcap\n");
		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		printf("uv_run exited\n");
		}

	return 0;
	}
