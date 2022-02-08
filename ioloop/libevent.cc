#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <strings.h>
#include <event2/event.h>

struct event_base* base;
pcap_t* pc;

void pcap_hdlr(unsigned char* user, const struct pcap_pkthdr* h, const unsigned char* bytes)
	{
	printf("packet of length %d\n", h->len);
	}

void cb(int fd, short what, void* arg)
	{
	printf("callback\n");
	if ( pcap_dispatch(pc, 1, pcap_hdlr, NULL) == 0 )
		{
		printf("ran out of data\n");
		struct event* me = reinterpret_cast<struct event*>(arg);
		event_del(me);
		}
	else
		{
		}
	}

int main(int argc, char **argv)
	{
	printf("App Started\n");

	char errbuf[PCAP_ERRBUF_SIZE];
	pc = pcap_open_offline(argv[1], errbuf);

	base = event_base_new();

    printf("Using Libevent with backend method %s.\n",
        event_base_get_method(base));

	struct event* evt;
	evt = event_new(base, fileno(pcap_file(pc)), EV_READ | EV_PERSIST, cb, event_self_cbarg());
	event_add(evt, 0);

	event_base_dispatch(base);

	printf("App Ended\n");
	}
