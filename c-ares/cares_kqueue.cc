#include <sys/event.h>
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
#include <sys/time.h>

#include <set>

#include <ares.h>
#include <ares_nameser.h>

int queue;
int active_events = 0;
std::set<int> active_read_fds;
std::set<int> active_write_fds;

static void sock_cb(void* data, int s, int read, int write)
	{
	int index = 0;
	struct kevent events[2];

	if (read == 1 && active_read_fds.count(s) == 0)
		{
		EV_SET(&events[index], s, EVFILT_READ, EV_ADD, 0, 0, NULL);
		++index;
		active_read_fds.insert(s);
		}
	else if ( read == 0 && active_read_fds.count(s) != 0)
		{
		EV_SET(&events[index], s, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		++index;
		active_read_fds.erase(s);
		}

	if (write == 1 && active_write_fds.count(s) == 0)
		{
		EV_SET(&events[index], s, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
		active_write_fds.insert(s);
		++index;
		}
	else if (write == 0 && active_write_fds.count(s) != 0)
		{
		EV_SET(&events[index], s, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		++index;
		active_write_fds.erase(s);
		}

	int ret = kevent(queue, events, index, NULL, 0, NULL);
	if (ret == -1)
		perror("failed to initialize new events");
	}

static void query_ptr_callback(void *arg, int status, int timeouts, unsigned char* buf, int len)
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
		printf("Failed to parse PTR record %s\n", ares_strerror(r));
		return;
		}

	printf("query_ptr_callback parsed host: %s\n", host->h_name);
	ares_free_hostent(host);
	}

static void wait_ares(ares_channel channel)
	{
	for(;;)
		{
		struct timespec ts;
		struct timeval *tvp, tv;
		tvp = ares_timeout(channel, NULL, &tv);
		TIMEVAL_TO_TIMESPEC(&tv, &ts);

		size_t num_events = active_read_fds.size() + active_write_fds.size();
		std::unique_ptr<struct kevent[]> events(new struct kevent[num_events]);
		int ret = kevent(queue, NULL, 0, events.get(), num_events, &ts);

		if (ret == -1)
			{
			perror("kevent failed");
			break;
			}
		else if (ret == 0)
			{
			break;
			}
		else
			{
			for (int i = 0; i < ret; i++)
				{
				int read = ARES_SOCKET_BAD;
				int write = ARES_SOCKET_BAD;
				if (events[i].filter == EVFILT_READ)
					read = events[i].ident;
				else if (events[i].filter == EVFILT_WRITE)
					write = events[i].ident;

				ares_process_fd(channel, read, write);
				}
			}
		}
	}

int main(void)
	{
	queue = kqueue();
	if (queue == -1)
		{
		perror("Failed to open queue");
		exit(1);
		}

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

	options.tries = 2;
	optmask |= ARES_OPT_TRIES;

	options.timeout = 5 * 1000;
	optmask |= ARES_OPT_TIMEOUTMS;

	options.sock_state_cb = sock_cb;
	optmask |= ARES_OPT_SOCK_STATE_CB;

	status = ares_init_options(&channel, &options, optmask);
	if(status != ARES_SUCCESS)
		{
		printf("ares_init_options: %s\n", ares_strerror(status));
		return 1;
		}

	struct ares_addr_node servers;
	servers.next = NULL;
	servers.family = AF_INET;
	inet_pton(AF_INET, "1.1.1.1", &(servers.addr.addr4));
	ares_set_servers(channel, &servers);

	unsigned char* query = NULL;
	int len = 0;
	status = ares_create_query("10.144.253.17.in-addr.arpa", C_IN, T_PTR, 0x0000,
	                           1, &query, &len, 0);
	if (status != ARES_SUCCESS)
		{
		printf("ares_create_query: %s\n", ares_strerror(status));
		return 1;
		}

	ares_send(channel, query, len, query_ptr_callback, NULL);
	wait_ares(channel);
	ares_free_string(query);

	ares_destroy(channel);
	ares_library_cleanup();
	return 0;
}
