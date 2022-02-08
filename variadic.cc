#include <utility>
#include <stdio.h>

template <typename... Args>
void foo(const char* fmt, Args&&... args)
	{
	constexpr size_t size = sizeof...(args);

	if constexpr ( sizeof...(args) > 0 )
		printf(fmt, std::forward<Args>(args)...);
	else
		printf("%s", fmt);
	}

int main(int argc, char** argv)
	{
	foo("test\n");
	foo("%d\n", 5);
	}
