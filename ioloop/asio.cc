#include <cstdio>
#include <asio.hpp>
#include <pcap.h>

pcap_t* pd;
asio::posix::stream_descriptor* input;

void pcap_hdlr(u_char* user, const struct pcap_pkthdr* h, const u_char* bytes)
	{
	printf("got packet of length %d\n", h->len);
	}

void wait_handler(asio::error_code ec)
	{
	printf("wait handler: ec = %s\n", ec.message().c_str());
	if ( ! ec )
		{
		struct pcap_pkthdr hdr;
		int dispatch = pcap_dispatch(pd, 1, pcap_hdlr, NULL);
		printf("dispatch returned %d\n", dispatch);
		if ( dispatch == 0 )
			printf("out of data?\n");
		else
			input->async_wait(asio::posix::stream_descriptor::wait_read, wait_handler);
		}
	}

void read_handler(const asio::error_code& ec, std::size_t bytes_transferred)
	{
	printf("wait handler: ec = %s\n", ec.message().c_str());
	if ( ! ec )
		{
		struct pcap_pkthdr hdr;
		int dispatch = pcap_dispatch(pd, 1, pcap_hdlr, NULL);
		printf("dispatch returned %d\n", dispatch);
		if ( dispatch == 0 )
			printf("out of data?\n");
		else
			input->async_read_some(asio::null_buffers(), read_handler);
		}
	}

int main(int argc, char** argv)
	{
	char errbuf[PCAP_ERRBUF_SIZE];
	pd = pcap_open_offline(argv[1], errbuf);

	if ( ! pd )
		{
		fprintf(stderr, "%s\n", errbuf);
		exit(1);
		}

	asio::io_context io;
	input = new asio::posix::stream_descriptor(io, fileno(pcap_file(pd)));
//	input->async_wait(asio::posix::stream_descriptor::wait_read, wait_handler);
	input->async_read_some(asio::null_buffers(), read_handler);
	io.run();
	}
