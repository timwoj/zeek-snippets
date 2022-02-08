#include <ares.h>
#include <ares_nameser.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

static void state_cb(void *data, int s, int read, int write)
	{
	printf("Change state fd %d read:%d write:%d\n", s, read, write);
	}


static void query_a_callback(void *arg, int status, int timeouts, unsigned char* buf, int len)
	{
	if (status != ARES_SUCCESS)
		{
		printf("Failed to lookup %s\n", ares_strerror(status));
		return;
		}

	struct hostent* host;
	int r = ares_parse_a_reply(buf, len, &host, NULL, NULL);
	if ( r != ARES_SUCCESS )
		{
		printf("Failed to parse A record %s\n", ares_strerror(r));
		return;
		}

	printf("query_a_callback parsed host: %s\n", host->h_name);
	ares_free_hostent(host);
	}

static void ptr_callback(void *arg, int status, int timeouts, unsigned char* buf, int len)
	{
	if (status != ARES_SUCCESS)
		{
		printf("Failed to lookup %s\n", ares_strerror(status));
		return;
		}

	uint8_t addrv4[4];
	struct hostent* host;
	int r = ares_parse_ptr_reply(buf, len, addrv4, sizeof(addrv4), AF_INET, &host);
	if ( r != ARES_SUCCESS )
		{
		printf("Failed to parse NS record %s\n", ares_strerror(r));
		return;
		}

	printf("ptr_callback parsed host: %s\n", host->h_name);
	ares_free_hostent(host);
	}

static void wait_ares(ares_channel channel)
	{
	for(;;)
		{
		struct timeval *tvp, tv;
		fd_set read_fds, write_fds;
		int nfds;

		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		nfds = ares_fds(channel, &read_fds, &write_fds);
		if(nfds == 0)
			{
			break;
			}
		tvp = ares_timeout(channel, NULL, &tv);
		select(nfds, &read_fds, &write_fds, NULL, tvp);
		ares_process(channel, &read_fds, &write_fds);
		}
	}

static int ares_socket_cb(ares_socket_t fd, int type, void* user_data)
	{
	return ARES_SUCCESS;
	}

int main(void)
	{
	ares_channel channel;
	int status;
	struct ares_options options;
	int optmask = 0;

	status = ares_library_init(ARES_LIB_INIT_ALL);
	if (status != ARES_SUCCESS)
		{
		printf("ares_library_init: %s\n", ares_strerror(status));
		return 1;
		}

	options.flags = ARES_FLAG_STAYOPEN;
	optmask |= ARES_OPT_FLAGS;

	options.sock_state_cb = state_cb;
	optmask |= ARES_OPT_SOCK_STATE_CB;

	status = ares_init_options(&channel, &options, optmask);
	if(status != ARES_SUCCESS)
		{
		printf("ares_init_options: %s\n", ares_strerror(status));
		return 1;
		}

    ares_set_socket_callback(channel, ares_socket_cb, NULL);

    /* status = ares_create_query("google.com", C_IN, T_A, 0x0000, 0, &query, &len, 0); */
    /* if (status != ARES_SUCCESS) */
	/*     { */
	/*     printf("ares_create_query: %s\n", ares_strerror(status)); */
	/*     return 1; */
	/*     } */

    /* ares_send(channel, query, len, query_a_callback, NULL); */
    /* wait_ares(channel); */
    /* ares_free_string(query); */

    ares_query(channel, "1.1.1.1.in-addr.arpa", C_IN, T_PTR, ptr_callback, NULL);
    wait_ares(channel);

    ares_destroy(channel);
    ares_library_cleanup();
    return 0;
}
