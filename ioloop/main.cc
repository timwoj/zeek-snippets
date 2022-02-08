#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <uv.h>
#include <pcap.h>

pcap_t* pd;
uv_poll_t poller;
uv_idle_t idle;
int fd;

void pcap_hdlr(u_char* user, const struct pcap_pkthdr* h, const u_char* bytes)
	{
	printf("got packet of length %d\n", h->len);
	}

void pcap_poll_callback(uv_poll_t* handle, int status, int events)
	{
	struct pcap_pkthdr hdr;
	int loop = pcap_dispatch(pd, 1, pcap_hdlr, NULL);
	printf("(status = %d, events = %d) loop returned %d\n", status, events, loop);
	if ( loop == 0 )
		{
		printf("out of data?\n");
		uv_poll_stop(&poller);
		}
	}

void pcap_idle_callback(uv_idle_t* handle)
	{
	struct pcap_pkthdr hdr;
	int dispatch = pcap_dispatch(pd, 1, pcap_hdlr, NULL);
	printf("dispatch returned %d\n", dispatch);
	if ( dispatch == 0 )
		{
		printf("out of data?\n");
		uv_idle_stop(&idle);
		}
	}

void file_callback(uv_poll_t* handle, int status, int events)
	{
	printf("%d %d\n", status, events);
	unsigned char data[500];
	int bytes = read(fd, data, 500);
	if ( bytes == 1 )
		{
		printf("error %s\n", strerror(errno));
		uv_poll_stop(&poller);
		}
	else if ( bytes == 0 )
		{
		printf("eof\n");
		uv_poll_stop(&poller);
		}
	else
		{
		printf("read %d bytes\n", bytes);
		}
	}

bool init_pcap(uv_loop_t* loop, char* path)
	{
	char errbuf[PCAP_ERRBUF_SIZE];
	pd = pcap_open_offline(path, errbuf);

	if ( ! pd )
		{
		fprintf(stderr, "%s\n", errbuf);
		return false;
		}

	if ( uv_poll_init(loop, &poller, fileno(pcap_file(pd))) == 0 )
		{
		printf("calling uv_poll_start for pcap\n");
		uv_poll_start(&poller, UV_READABLE, pcap_poll_callback);
		}

	return true;
	}

bool init_file(uv_loop_t* loop, char* path)
	{
	fd = open(path, O_RDONLY | O_NONBLOCK);
	printf("%d\n", fd);
	if ( fd == -1 )
		{
		fprintf(stderr, "%s", strerror(errno));
		return false;
		}

	if ( uv_poll_init(loop, &poller, fd) == 0 )
		{
		printf("calling uv_poll_start for file\n");
		uv_poll_start(&poller, UV_READABLE | UV_DISCONNECT, file_callback);
		}

	return true;
	}

bool init_idle(uv_loop_t* loop, char* path)
	{
	char errbuf[PCAP_ERRBUF_SIZE];
	pd = pcap_open_offline(path, errbuf);

	if ( ! pd )
		{
		fprintf(stderr, "%s\n", errbuf);
		return false;
		}

	if ( uv_idle_init(loop, &idle) == 0 )
		{
		printf("calling uv_idle_start for pcap\n");
		uv_idle_start(&idle, pcap_idle_callback);
		}

	return true;
	}

bool init_interface(uv_loop_t* loop, char* iface)
	{
	char errbuf[PCAP_ERRBUF_SIZE];
	pd = pcap_open_live(iface, 9216, 1, 1, errbuf);

	if ( ! pd )
		{
		fprintf(stderr, "failed to open interface: %s\n", errbuf);
		return false;
		}

	pcap_set_buffer_size(pd, 128 * 1024 * 1024);

	int init = uv_poll_init(loop, &poller, pcap_get_selectable_fd(pd));
	if ( init == 0 )
		{
		pcap_activate(pd);
		pcap_setnonblock(pd, 1, errbuf);
		
		printf("calling uv_poll_start for pcap\n");
		uv_poll_start(&poller, UV_READABLE, pcap_poll_callback);
		return true;
		}
	else
		{
		printf("Failed to create uv_poll: %s\n", uv_strerror(init));
		return false;
		}

	return true;
	}

int main(int argc, char** argv)
	{
	uv_loop_t loop;
	uv_loop_init(&loop);
	if ( strcmp(argv[1], "p") == 0 )
		{
		if ( init_pcap(&loop, argv[2]) )
			{
			printf("calling uv_run for pcap\n");
			uv_run(&loop, UV_RUN_DEFAULT);
			printf("uv_run exited\n");
			}
		}
	else if ( strcmp(argv[1], "f") == 0 )
		{
		if ( init_file(&loop, argv[2]) )
			{
			printf("calling uv_run for file\n");
			uv_run(&loop, UV_RUN_DEFAULT);
			printf("uv_run exited\n");
			}
		}
	else if ( strcmp(argv[1], "i") == 0 )
		{
		if ( init_idle(&loop, argv[2]) )
			{
			printf("calling uv_run for idle loop\n");
			uv_run(&loop, UV_RUN_DEFAULT);
			}
		}
	else if ( strcmp(argv[1], "if") == 0 )
		{
		if ( init_interface(&loop, argv[2]) )
			{
			printf("calling uv_run for interface poll loop\n");
			uv_run(&loop, UV_RUN_DEFAULT);
			}
		}
	return 0;
	}
