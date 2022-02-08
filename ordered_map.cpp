#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <map>

struct ConnIDKey
	{
	in6_addr ip1;
	in6_addr ip2;
	uint16_t port1;
	uint16_t port2;

	ConnIDKey() : port1(0), port2(0)
		{
		memset(&ip1, 0, sizeof(in6_addr));
		memset(&ip2, 0, sizeof(in6_addr));
		}

	bool operator<(const ConnIDKey& rhs) const
		{
		printf("operator<\n");
		log();
		rhs.log();
		return ( (port1 < rhs.port1) ||
		         (port2 < rhs.port2) ||
		         memcmp(&ip1, &rhs.ip1, sizeof(in6_addr)) < 0 ||
		         memcmp(&ip2, &rhs.ip2, sizeof(in6_addr)) < 0 );
		}

	ConnIDKey& operator=(const ConnIDKey& rhs)
		{
		port1 = rhs.port1;
		port2 = rhs.port2;
		memcpy(&ip1, &rhs.ip1, sizeof(in6_addr));
		memcpy(&ip2, &rhs.ip2, sizeof(in6_addr));
		return *this;
		}

	void log() const
		{
		char ip1str[INET6_ADDRSTRLEN];
		char ip2str[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &ip1, ip1str, INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &ip2, ip2str, INET6_ADDRSTRLEN);
		printf("  %s:%d  %s:%d\n", ip1str, port1, ip2str, port2);
		}
	};

int main(int argc, char** argv)
	{
	std::map<ConnIDKey, int> h;

	ConnIDKey key1;
	key1.port1 = 13568;
	key1.port2 = 22420;
	inet_pton(AF_INET6, "::ffff:172.16.238.1", &key1.ip1);
	inet_pton(AF_INET6, "::ffff:224.0.0.251", &key1.ip2);
	h[key1] = 12345;

	// ConnIDKey key2;
	// key2.port1 = 13568;
	// key2.port2 = 22420;
	// inet_pton(AF_INET6, "::ffff:172.16.238.1", &key2.ip1);
	// inet_pton(AF_INET6, "::ffff:224.0.0.251", &key2.ip2);
	// auto it = h.find(key2);
	// if (it != h.end())
	// 	printf("found it\n");
	}
