#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/event.h>
#include <fcntl.h>
#include <unistd.h>
#include <pcap.h>

pcap_t* pc;

int main(int argc, char** argv)
	{
	struct pcap_pkthdr current_hdr = {};
	char errbuf[PCAP_ERRBUF_SIZE];
	pc = pcap_open_offline(argv[1], errbuf);
	if ( pcap_setnonblock(pc, 1, errbuf) )
		printf("%s\n", errbuf);

	struct kevent ke;
	off_t nread = 0;

	int kq = kqueue();
	int fd = pcap_get_selectable_fd(pc);

	struct timespec timeout = {0, 0};

	EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
	kevent(kq, &ke, 1, NULL, 0, NULL);
	int count = 0;
	int packet_count = 0;

	while (1)
		{
		off_t left;

		memset(&ke, 0, sizeof(struct kevent));
		int ret = kevent(kq, NULL, 0, &ke, 1, &timeout);
		if ( ret == -1 )
			{
			perror("kevent failed");
			break;
			}
		else if ( ret == 0 )
			{
			printf("timeout\n");
			count++;

			if ( count == 50 )
				break;

			printf("%lu %d\n", ke.ident, ke.filter);
			}
		else
			{
			if( ke.flags & EV_ERROR )
				{
				printf("flags had error\n");
				break;
				}

			if (ke.flags & EV_EOF)
				{
				printf("premature end of file!\n");
				break;
				}

			packet_count++;
			const u_char* data = pcap_next(pc, &current_hdr);
			printf("packet %d of length %d\n", packet_count, current_hdr.len);

			if ( ! data )
				break;
			}
		}
	return 0;
	}
