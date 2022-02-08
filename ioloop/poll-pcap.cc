#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <pcap.h>

void pcap_hdlr(unsigned char* user, const struct pcap_pkthdr* h, const unsigned char* bytes)
	{
	printf("packet of length %d\n", h->len);
	}

int main(int argc, char **argv)
	{
	char buff[500];
	int buff_sz;                             //size of data recieved

	printf("App Started\n");

	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *pc = pcap_open_offline(argv[1], errbuf);

	struct pollfd pd;
	memset(&pd, 0, sizeof(struct pollfd));
	pd.fd = fileno(pcap_file(pc));
	pd.events = POLLIN;
	pd.revents = 0;

	while (1)
		{
		pd.events = POLLIN | POLLPRI;
		pd.revents = 0;

		//put all this into poll and wait for something magical to happen
		printf("calling poll\n");
		if (poll(&pd, 1, 0) == -1)
			{
			perror("poll");
			exit(0);
			}

		printf("poll returned!\n");

		int loop = pcap_dispatch(pc, 1, pcap_hdlr, NULL);
		if ( loop == 0 )
			{
			printf("out of data\n");
			break;
			}
		}

	printf("App Ended\n");
	}
