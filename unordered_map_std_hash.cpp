#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unordered_map>
#include <functional>

class ConnIDKey
	{
	in6_addr ip1;
	in6_addr ip2;
	uint16_t port1;
	uint16_t port2;
	bool valid = false;

	// ConnIDKey() : port1(0), port2(0), valid(false) {}
	// ConnIDKey(const ConnIDKey& c) = default;
	// ConnIDKey(ConnIDKey&& c) = default;

	bool operator<(const ConnIDKey& rhs) const
		{
		return ( (port1 < rhs.port1) ||
			(port2 < rhs.port2) ||
			memcmp(ip1.s6_addr, rhs.ip1.s6_addr, 16) < 0 ||
			memcmp(ip2.s6_addr, rhs.ip2.s6_addr, 16) < 0 );
		}
	};

namespace std
	{
	template<> struct hash<ConnIDKey>
		{
		typedef ConnIDKey argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const noexcept
			{
			return 0;
			}
		};
	}

int main(int argc, char** argv)
	{
	std::unordered_map<ConnIDKey, int> h;
	}
