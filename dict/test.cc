#include <map>
#include <unordered_map>
#include <stdio.h>
#include <time.h>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>

#include "OpenDict.h"
#include "fifo_map.hpp"

void regular()
	{
	fprintf(stderr, "std::map\n");
	std::map<int, int> m;

	struct timespec ts;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int i = 0; i < 1000000; i++ )
		{
		m.emplace(i, i);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
//		printf("%d,%llu\n", i, now-last);
		last = now;
		}

	fprintf(stderr, "Final size: %lu\n", m.size());
	}

void unordered()
	{
	fprintf(stderr, "std::unordered_map\n");
	std::unordered_map<int, int> m;

	struct timespec ts;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int i = 0; i < 1000000; i++ )
		{
		m.emplace(i, i);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
		printf("%d,%llu\n", i, now-last);
		last = now;
		}

	fprintf(stderr, "Final size: %lu\n", m.size());
	}

void fifo()
	{
	fprintf(stderr, "nlohmann::fifo_map\n");
	nlohmann::fifo_map<int, int> m;

	struct timespec ts;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int i = 0; i < 1000000; i++ )
		{
		m.emplace(i, i);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
//		printf("%d,%llu\n", i, now-last);
		last = now;
		}

	fprintf(stderr, "Final size: %lu\n", m.size());
	}

void dict()
	{
	fprintf(stderr, "Dictionary\n");
	PDict<int> d;

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int i = 0; i < 1000000; i++ )
		{
		HashKey* h = new HashKey(bro_int_t(i));
		d.Insert(h, new int(i));
		clock_gettime(CLOCK_REALTIME, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
		printf("%d,%llu\n", i, now-last);
		last = now;
		}

	fprintf(stderr, "Final size: %d\n", d.Length());
	}

void create_regular()
	{
	fprintf(stderr, "Create std::map\n");

	struct timespec ts;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int i = 0; i < 1000000; i++ )
		{
		std::map<int, int*> m;
		m.emplace(0, new int(0));
		m.emplace(1, new int(1));
		m.emplace(1, new int(2));

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
		printf("%d,%llu\n", i, now-last);
		last = now;
		}
	}

void create_dict()
	{
	fprintf(stderr, "Create Dictionary\n");

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int i = 0; i < 1000000; i++ )
		{
		PDict<int> d;
 		HashKey* h1 = new HashKey(bro_int_t(0));
		d.Insert(h1, new int(0));
 		HashKey* h2 = new HashKey(bro_int_t(1));
		d.Insert(h2, new int(1));
 		HashKey* h3 = new HashKey(bro_int_t(2));
		d.Insert(h3, new int(2));
		
		clock_gettime(CLOCK_REALTIME, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
		printf("%d,%llu\n", i, now-last);
		last = now;
		}
	}

const int LOOKUP_LENGTH = 50;

void lookup_map()
	{
	fprintf(stderr, "std::map lookup\n");
	std::string keys[LOOKUP_LENGTH];
	
	std::map<const char*, int*> m;
	for (int i = 0; i < LOOKUP_LENGTH; i++)
		{
		keys[i] = std::to_string(i);
		m[keys[i].c_str()] = new int(i);
		}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t start = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for (int i = LOOKUP_LENGTH-1; i >= 0; i--)
		{
		int* result = m[keys[i].c_str()];
		}

	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t end = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	printf("Time for %d lookups (us): %llu\n", LOOKUP_LENGTH, (end - start));
	}

void lookup_unordered()
	{
	fprintf(stderr, "std::unordered_map lookup\n");
	std::string keys[LOOKUP_LENGTH];
	
	std::unordered_map<const char*, int*> m;
	for (int i = 0; i < LOOKUP_LENGTH; i++)
		{
		keys[i] = std::to_string(i);
		m[keys[i].c_str()] = new int(i);
		}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t start = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for (int i = LOOKUP_LENGTH-1; i >= 0; i--)
		{
		int* result = m[keys[i].c_str()];
		}

	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t end = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	printf("Time for %d lookups (us): %llu\n", LOOKUP_LENGTH, (end - start));
	}

void lookup_fifo()
	{
	fprintf(stderr, "nlohmann::fifo_map lookup\n");
	std::string keys[LOOKUP_LENGTH];
	
	nlohmann::fifo_map<const char*, int*> m;
	for (int i = 0; i < LOOKUP_LENGTH; i++)
		{
		keys[i] = std::to_string(i);
		m[keys[i].c_str()] = new int(i);
		}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t start = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for (int i = LOOKUP_LENGTH-1; i >= 0; i--)
		{
		int* result = m[keys[i].c_str()];
		}

	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t end = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	printf("Time for %d lookups (us): %llu\n", LOOKUP_LENGTH, (end - start));
	}

void lookup_dict()
	{
	fprintf(stderr, "Dictionary lookup\n");
	std::string keys[LOOKUP_LENGTH];
	
	PDict<int> d;
	for (int i = 0; i < LOOKUP_LENGTH; i++)
		{
		keys[i] = std::to_string(i);
		d.Insert(new HashKey(keys[i].c_str()), new int(i));
		}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t start = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for (int i = LOOKUP_LENGTH-1; i >= 0; i--)
		{
		int* result = d.Lookup(keys[i].c_str());
		}

	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t end = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	printf("Time for %d lookups (us): %llu\n", LOOKUP_LENGTH, (end - start));
	}

void iterate_map()
	{
	fprintf(stderr, "std::map iterate\n");
	std::string keys[LOOKUP_LENGTH];
	
	std::map<const char*, int*> m;
	for (int i = 0; i < LOOKUP_LENGTH; i++)
		{
		keys[i] = std::to_string(i);
		m[keys[i].c_str()] = new int(i);
		}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t start = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( const auto& entry : m )
		{
		int* result = entry.second;
		}

	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t end = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	printf("Time for %d iterations (us): %llu\n", LOOKUP_LENGTH, (end - start));
	}

void iterate_unordered()
	{
	fprintf(stderr, "std::unordered_map iterate\n");
	std::string keys[LOOKUP_LENGTH];
	
	std::unordered_map<const char*, int*> m;
	for (int i = 0; i < LOOKUP_LENGTH; i++)
		{
		keys[i] = std::to_string(i);
		m[keys[i].c_str()] = new int(i);
		}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t start = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( const auto& entry : m )
		{
		int* result = entry.second;
		}

	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t end = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	printf("Time for %d iterations (us): %llu\n", LOOKUP_LENGTH, (end - start));
	}

void iterate_fifo()
	{
	fprintf(stderr, "nlohmann::fifo_map iterate\n");
	std::string keys[LOOKUP_LENGTH];
	
	nlohmann::fifo_map<const char*, int*> m;
	for (int i = 0; i < LOOKUP_LENGTH; i++)
		{
		keys[i] = std::to_string(i);
		m[keys[i].c_str()] = new int(i);
		}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t start = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( const auto& entry : m )
		{
		int* result = entry.second;
		}

	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t end = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	printf("Time for %d iterations (us): %llu\n", LOOKUP_LENGTH, (end - start));
	}

void iterate_dict()
	{
	fprintf(stderr, "Dictionary iterate\n");
	std::string keys[LOOKUP_LENGTH];
	
	PDict<int> d;
	for (int i = 0; i < LOOKUP_LENGTH; i++)
		{
		keys[i] = std::to_string(i);
		d.Insert(new HashKey(keys[i].c_str()), new int(i));
		}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t start = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	IterCookie* c = d.InitForIteration();
	int* entry;
	while ( (entry = d.NextEntry(c)) )
		{
		}

	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t end = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	printf("Time for %d iterations (us): %llu\n", LOOKUP_LENGTH, (end - start));
	}

struct ConnIDKey
	{
	in6_addr ip1;
	in6_addr ip2;
	uint16_t port1;
	uint16_t port2;

	bool operator<(const ConnIDKey& rhs) const
		{
		return memcmp(this, &rhs, sizeof(ConnIDKey)) < 0;
		}

	bool operator==(const ConnIDKey& rhs) const
		{
		return memcmp(this, &rhs, sizeof(ConnIDKey)) == 0;
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
			return HashKey::HashBytes(&s, sizeof(argument_type));
			}
		};
	}

void complex_regular()
	{
//	fprintf(stderr, "std::map complex key\n");
	std::map<ConnIDKey, int> m;

	struct timespec ts;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int64_t i = 0; i < 1000000; i++ )
		{
		ConnIDKey key;
		memset(&key, 0, sizeof(ConnIDKey));
		memcpy(&key.ip1, &i, sizeof(size_t));
		memcpy(&key.ip2, &i, sizeof(size_t));
		
		m.emplace(key, i);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
		printf("%lld,%llu\n", i, now-last);
		last = now;
		}

//	fprintf(stderr, "Final size: %lu\n", m.size());
	}

void complex_unordered()
	{
//	fprintf(stderr, "std::unordered_map complex key\n");
	std::unordered_map<ConnIDKey, int> m;

	struct timespec ts;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int64_t i = 0; i < 1000000; i++ )
		{
		ConnIDKey key;
		memset(&key, 0, sizeof(ConnIDKey));
		memcpy(&key.ip1, &i, sizeof(size_t));
		memcpy(&key.ip2, &i, sizeof(size_t));
		
		m.emplace(key, i);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
		printf("%lld,%llu\n", i, now-last);
		last = now;
		}

//	fprintf(stderr, "Final size: %lu\n", m.size());
	}

void complex_dict()
	{
//	fprintf(stderr, "Dictionary\n");
	PDict<int> d;

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	uint64_t last = (ts.tv_sec * 1000000000) + ts.tv_nsec;

	for ( int64_t i = 0; i < 1000000; i++ )
		{
		ConnIDKey key;
		memset(&key, 0, sizeof(ConnIDKey));
		memcpy(&key.ip1, &i, sizeof(size_t));
		memcpy(&key.ip2, &i, sizeof(size_t));

		HashKey* h = new HashKey(&key, sizeof(ConnIDKey));
		d.Insert(h, new int(i));
		clock_gettime(CLOCK_REALTIME, &ts);
		uint64_t now = (ts.tv_sec * 1000000000) + ts.tv_nsec;
		printf("%lld,%llu\n", i, now-last);
		last = now;
		}

//	fprintf(stderr, "Final size: %d\n", d.Length());
	}

int main(int argc, char** argv)
	{
	if (argc != 2)
		return -1;
	
	if (argv[1][0] == 'u')
		unordered();
	else if (argv[1][0] == 'r')
		regular();
	else if (argv[1][0] == 'd')
		dict();
	else if (argv[1][0] == 'f')
		fifo();
	else if (strcmp(argv[1], "cr") == 0)
		create_regular();
	else if (strcmp(argv[1], "cd") == 0)
		create_dict();
	else if (strcmp(argv[1], "lr") == 0)
		lookup_map();
	else if (strcmp(argv[1], "lu") == 0)
		lookup_unordered();
	else if (strcmp(argv[1], "lf") == 0)
		lookup_fifo();
	else if (strcmp(argv[1], "ld") == 0)
		lookup_dict();
	else if (strcmp(argv[1], "ir") == 0)
		iterate_map();
	else if (strcmp(argv[1], "iu") == 0)
		iterate_unordered();
	else if (strcmp(argv[1], "if") == 0)
		iterate_fifo();
	else if (strcmp(argv[1], "id") == 0)
		iterate_dict();
	else if (strcmp(argv[1], "cpr") == 0)
		complex_regular();
	else if (strcmp(argv[1], "cpu") == 0)
		complex_unordered();
	else if (strcmp(argv[1], "cpd") == 0)
		complex_dict();
	}
