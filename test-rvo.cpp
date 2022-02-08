#include <stdio.h>
#include <map>

struct A
	{
	int a;
	int b;

	A() { printf("const\n"); }
	~A() { printf("dest\n"); }
	A(const A& a) { printf("copy\n"); }
	A operator=(const A& a) { printf("assign\n"); return *this; }
	bool operator<(const A& a) const { printf("less\n"); return false; }
	bool operator==(const A& a) const { printf("eq\n"); return true; }
	};

A foo()
	{
	A a;
	return a;
	}

int main(int argc, char** argv)
	{
	A a = foo();
	printf("1\n");
	std::map<A, int> m;
	m[a] = 0;
	printf("2\n");
	auto it = m.find(a);
	printf("3\n");
	}
