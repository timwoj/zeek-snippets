#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/event.h>
#include <fcntl.h>
#include <unistd.h>
#include <pcap.h>
#include <vector>

pcap_t* pd;

int OpenIntf(const char* intf)
	{
	char errbuf[PCAP_ERRBUF_SIZE];
	char tmp_errbuf[PCAP_ERRBUF_SIZE];

	// Determine network and netmask.
	uint32_t net;
	uint32_t netmask = 0xffffffff;
	if ( pcap_lookupnet(intf, &net, &netmask, tmp_errbuf) < 0 )
		{
		// ### The lookup can fail if no address is assigned to
		// the interface; and libpcap doesn't have any useful notion
		// of error codes, just error std::strings - how bogus - so we
		// just kludge around the error :-(.
		// sprintf(errbuf, "pcap_lookupnet %s", tmp_errbuf);
		// return;
		netmask = 0xffffff00;
		}

#ifdef PCAP_NETMASK_UNKNOWN
	// Defined in libpcap >= 1.1.1
	if ( netmask == PCAP_NETMASK_UNKNOWN )
		netmask = 0xffffffff;
#endif

	pd = pcap_create(intf, errbuf);

	if ( ! pd )
		{
		printf("pcap_create: %s\n", pcap_geterr(pd));
		return -1;
		}

	if ( pcap_set_snaplen(pd, 9216) )
		{
		printf("pcap_set_snaplen: %s\n", pcap_geterr(pd));
		return -1;
		}

	if ( pcap_set_promisc(pd, 1) )
		{
		printf("pcap_set_promisc: %s\n", pcap_geterr(pd));
		return -1;
		}

	// We use the smallest time-out possible to return almost immediately
	// if no packets are available. (We can't use set_nonblocking() as
	// it's broken on FreeBSD: even when select() indicates that we can
	// read something, we may get nothing if the store buffer hasn't
	// filled up yet.)
	//
	// TODO: The comment about FreeBSD is pretty old and may not apply
	// anymore these days.
	if ( pcap_set_timeout(pd, 1) )
		{
		printf("pcap_set_timeout: %s\n", pcap_geterr(pd));
		return -1;
		}

	if ( pcap_set_buffer_size(pd, 128 * 1024 * 1024) )
		{
		printf("pcap_set_buffer_size: %s\n", pcap_geterr(pd));
		return -1;
		}

	if ( pcap_activate(pd) )
		{
		printf("pcap_activate: %s\n", pcap_geterr(pd));
		return -1;
		}

#ifdef HAVE_LINUX
	if ( pcap_setnonblock(pd, 1, tmp_errbuf) < 0 )
		{
		printf("pcap_setnonblock: %s\n", pcap_geterr(pd));
		return -1;
		}
#endif

	return pcap_get_selectable_fd(pd);
	}

int main(int argc, char** argv)
	{
	printf("Using pcap library %s\n", pcap_lib_version());

	struct pcap_pkthdr current_hdr = {};
	struct kevent ke;

	int kq = kqueue();
	int fd = OpenIntf("en0");

	struct timespec timeout;
	timeout.tv_sec = 1;
	timeout.tv_nsec = 0;

	printf("Using fd %d\n", fd);

	EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, NULL, NULL);
	kevent(kq, &ke, 1, NULL, 0, NULL);

	std::vector<struct kevent> events;
	struct kevent ke2;
	events.push_back(ke2);

	while (1)
		{
		off_t left;

		int ret = kevent(kq, NULL, 0, events.data(), events.size(), &timeout);
		if ( ret == -1 )
			perror("kevent failed");
		else if ( ret == 0 )
			printf("timeout\n");
		else
			{
			printf("%d interfaces ready\n", ret);

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

			const u_char* data = pcap_next(pd, &current_hdr);
			printf("packet of length %d\n", current_hdr.len);
			}
		}

	return 0;
	}
