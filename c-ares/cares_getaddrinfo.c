#include <ares.h>
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

static void
state_cb(void *data, int s, int read, int write)
{
    printf("Change state fd %d read:%d write:%d\n", s, read, write);
}

static void
addrinfo_cb(void* arg, int status, int timeouts, struct ares_addrinfo* result)
{
	if ( status != ARES_SUCCESS )
		{
		// TODO: reporter warning or something here, or just give up on it?
		printf("Failed addrinfo request: %s\n", ares_strerror(status));
		ares_freeaddrinfo(result);
		return;
		}

	struct ares_addrinfo_node* entry;
	for ( entry = result->nodes; entry != NULL; entry = entry->ai_next )
		{
		if ( entry->ai_family == AF_INET )
			{
			struct sockaddr_in* addr = (struct sockaddr_in*)(entry->ai_addr);
			char buf[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &addr->sin_addr, buf, sizeof(buf));
			printf("found new ipv4 address %s\n", buf);
			}
		else if ( entry->ai_family == AF_INET6 )
			{
			struct sockaddr_in6* addr = (struct sockaddr_in6*)(entry->ai_addr);
			char buf[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &addr->sin6_addr, buf, sizeof(buf));
			printf("found new ipv6 address %s\n", buf);
			}
		}

	ares_freeaddrinfo(result);
}

static void
wait_ares(ares_channel channel)
{
    for(;;){
        struct timeval *tvp, tv;
        fd_set read_fds, write_fds;
        int nfds;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        nfds = ares_fds(channel, &read_fds, &write_fds);
        if(nfds == 0){
            break;
        }
        tvp = ares_timeout(channel, NULL, &tv);
        select(nfds, &read_fds, &write_fds, NULL, tvp);
        ares_process(channel, &read_fds, &write_fds);
    }
}

static int ares_socket_cb(ares_socket_t fd, int type, void* user_data)
	{
	printf("%d %d\n", fd, type);
	return ARES_SUCCESS;
	}

int
main(void)
{
    ares_channel channel;
    int status;
    struct ares_options options;
    int optmask = 0;

    status = ares_library_init(ARES_LIB_INIT_ALL);
    if (status != ARES_SUCCESS){
        printf("ares_library_init: %s\n", ares_strerror(status));
        return 1;
    }
    printf("init done\n");

	options.flags = ARES_FLAG_STAYOPEN;
	optmask |= ARES_OPT_FLAGS;

    options.sock_state_cb = state_cb;
    optmask |= ARES_OPT_SOCK_STATE_CB;

    status = ares_init_options(&channel, &options, optmask);
    if(status != ARES_SUCCESS) {
        printf("ares_init_options: %s\n", ares_strerror(status));
        return 1;
    }
    printf("options done\n");

    ares_set_socket_callback(channel, ares_socket_cb, NULL);

    const char* host = "one.one.one.one";
    struct ares_addrinfo_hints hints = {ARES_AI_CANONNAME, AF_UNSPEC, 0, 0};
    ares_getaddrinfo(channel, host, NULL, &hints, addrinfo_cb, NULL);

    wait_ares(channel);
    ares_destroy(channel);
    ares_library_cleanup();

    return 0;
}
