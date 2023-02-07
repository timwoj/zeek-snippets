#include <sys/time.h>
#include <chrono>
#include <iostream>

int main(int argc, char** argv)
	{
	auto one = std::chrono::system_clock::now();

	struct timeval tv;
	for(int i = 0; i < 100000000; i++)
		{
		auto now = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
		tv.tv_sec = ms.count() / 1000;
		tv.tv_usec = (ms.count() % 1000) * 1000;
		}

	auto two = std::chrono::system_clock::now();

	for(int i = 0; i < 100000000; i++)
		gettimeofday(&tv, 0);

	auto three = std::chrono::system_clock::now();

	auto ch = std::chrono::duration_cast<std::chrono::milliseconds>(two-one);
	auto gtod = std::chrono::duration_cast<std::chrono::milliseconds>(three-two);

	std::cout << ch.count() << std::endl;
	std::cout << gtod.count() << std::endl;
	}
