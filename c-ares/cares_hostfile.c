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

static void
state_cb(void *data, int s, int read, int write)
{
    printf("Change state fd %d read:%d write:%d\n", s, read, write);
}


static void
hostbyname_cb(void *arg, int status, int timeouts, struct hostent *host)
{
    if(!host || status != ARES_SUCCESS){
        printf("Failed to lookup %s\n", ares_strerror(status));
        printf("timeouts: %d\n", timeouts);
        return;
    }

    printf("hostbyname: %s = ", host->h_name);
    char ip[INET6_ADDRSTRLEN];
    int i = 0;

    for (i = 0; host->h_addr_list[i]; ++i) {
        inet_ntop(host->h_addrtype, host->h_addr_list[i], ip, sizeof(ip));
        printf("%s ", ip);
    }
    printf("\n");
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

	printf("addrinfo: %s = ", result->name);
	struct ares_addrinfo_node* entry;
	for ( entry = result->nodes; entry != NULL; entry = entry->ai_next )
		{
		if ( entry->ai_family == AF_INET )
			{
			struct sockaddr_in* addr = (struct sockaddr_in*)(entry->ai_addr);
			char buf[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &addr->sin_addr, buf, sizeof(buf));
			printf("%s ", buf);
			}
		else if ( entry->ai_family == AF_INET6 )
			{
			struct sockaddr_in6* addr = (struct sockaddr_in6*)(entry->ai_addr);
			char buf[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &addr->sin6_addr, buf, sizeof(buf));
			printf("%s ", buf);
			}
		}

	printf("\n");

	ares_freeaddrinfo(result);
}

static void
hostbyaddr_cb(void *arg, int status, int timeouts, struct hostent *host)
{
    if(!host || status != ARES_SUCCESS){
        printf("Failed to lookup %s\n", ares_strerror(status));
        return;
    }

    if ( host->h_addrtype == AF_INET )
	    {
	    struct in_addr* addr = (struct in_addr*)(host->h_addr);
	    char buf[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET, addr, buf, sizeof(buf));
	    printf("hostbyaddr: %s = ", buf);
	    }
    else if ( host->h_addrtype == AF_INET6 )
	    {
	    struct in_addr6* addr = (struct in_addr6*)(host->h_addr);
	    char buf[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET6, addr, buf, sizeof(buf));
	    printf("hostbyaddr: %s = ", buf);
	    }

    printf("%s\n", host->h_name);
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
		}
	else
		{
		printf("query_a_callback parsed host: %s\n", host->h_name);
		ares_free_hostent(host);
		}
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

	options.flags = ARES_FLAG_STAYOPEN;
	optmask |= ARES_OPT_FLAGS;

    options.sock_state_cb = state_cb;
    optmask |= ARES_OPT_SOCK_STATE_CB;

    options.lookups = "fb";
    optmask |= ARES_OPT_LOOKUPS;

    status = ares_init_options(&channel, &options, optmask);
    if(status != ARES_SUCCESS) {
        printf("ares_init_options: %s\n", ares_strerror(status));
        return 1;
    }

    ares_set_socket_callback(channel, ares_socket_cb, NULL);

    ares_gethostbyname(channel, "google.com", AF_INET, hostbyname_cb, NULL);
    ares_gethostbyname(channel, "zeektest", AF_INET, hostbyname_cb, NULL);

    const char* host = "zeektest";
    struct ares_addrinfo_hints hints = {ARES_AI_CANONNAME, AF_UNSPEC, 0, 0};
    ares_getaddrinfo(channel, host, NULL, &hints, addrinfo_cb, NULL);

    struct sockaddr_in sa;
    inet_pton(AF_INET, "10.0.0.10", &(sa.sin_addr));
    ares_gethostbyaddr(channel, &sa.sin_addr, sizeof(sa.sin_addr), AF_INET, hostbyaddr_cb, NULL);
    wait_ares(channel);

    unsigned char* query = NULL;
    int len = 0;
    status = ares_create_query("zeektest", C_IN, T_A, 0x0000, 1, &query, &len, 0);
    if (status != ARES_SUCCESS)
	    {
	    printf("ares_create_query: %s\n", ares_strerror(status));
	    return 1;
	    }

    ares_send(channel, query, len, query_a_callback, NULL);
    wait_ares(channel);
    ares_free_string(query);

    ares_destroy(channel);
    ares_library_cleanup();
    return 0;
}
