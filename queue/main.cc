#include <stdio.h>
#include <sys/time.h>

#include <queue>
#include <functional>
#include <vector>
#include <random>
#include <algorithm>
#include <set>

#include "PriorityQueue.h"
#include "PriorityQueueUnrolled.h"
#include "PriorityQueueVect.h"
#include "prio_queue.hpp"

#include <absl/container/btree_set.h>
#include <phmap.h>
#include <btree.h>

std::vector<int> generate_randoms(int size)
	{
	std::vector<int> randoms(size);
	int index = 0;
	auto gen = [&index]() { return ++index; };
	std::generate(std::begin(randoms), std::end(randoms), gen);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(randoms.begin(), randoms.end(), g);

    return randoms;
	}

double time_diff(const struct timeval& start, const struct timeval& end)
	{
	double start_time = (double)start.tv_sec + ((double)start.tv_usec / 1000000);
	double end_time = (double)end.tv_sec + ((double)end.tv_usec / 1000000);
	return end_time - start_time;
	}

void diagnostics(const std::string& where, const struct timeval& start,
	const struct timeval& mid, const struct timeval& end,
	const std::vector<double>& stock = {}, const std::vector<double>& output = {},
	double stock_add = 0, double stock_remove = 0)
	{
	double add_time = time_diff(start, mid);
	double remove_time = time_diff(mid, end);

	if ( stock_add == 0 )
		{
		printf("%20s: add_time: %.6f%10s remove_time: %.6f%10s total: %.6f\n",
			"Stock", add_time, " ", remove_time, " ", add_time+remove_time);
		// printf("%s:  add_time: %.6f   remove_time: %.6f   total: %.6f\n",
		// 	where.c_str(), add_time, remove_time, add_time+remove_time);
		}
	else
		{
		if ( output.size() != stock.size() )
			{
			printf("output and stock differ in size\n");
			}
		else if ( ! output.empty() )
			{
			for ( int i = 0; i < output.size(); i++ )
				{
				if ( output[i] != stock[i] )
					{
					printf("index %d differs (%f and %f)\n", i, stock[i], output[i]);
					break;
					}
				}
			}

		printf("%20s: add_time: %.6f (%0.2fx)%2s remove_time: %.6f (%0.2fx)%2s total: %.6f (%0.2fx)\n",
			where.c_str(), add_time, add_time / stock_add, " ",
			remove_time, remove_time / stock_remove, " ",
			add_time + remove_time, (add_time + remove_time) / (stock_add + stock_remove));
		}
	}

class Comparer {
public:
	bool operator()(const PQ_Element* l, const PQ_Element* r) const noexcept
		{
		return l->Time() < r->Time();
		}
	};

using StdPQ = std::priority_queue<PQ_Element*, std::vector<PQ_Element*>, Comparer>;

void ordering()
	{
//	std::vector<int> values = generate_randoms(10);
	std::vector<int> values = { 9, 4, 8, 1, 6, 7, 2, 5, 10, 3 };
//	std::vector<int> values = { 5, 4, 3, 2, 1 };
//	std::vector<int> values = { 1, 2, 3, 4, 5 };
//	std::vector<int> values = { 1, 5, 3, 2, 4 };

	{
	PriorityQueue q;
	for ( auto v : values )
		{
		auto e = new PQ_Element(v);
		q.Add(e);
		}

	q.Output();
	auto e = q.Remove();
	}

	printf("\n");

	{
	std::multiset<PQ_Element*, Comparer> q;
	for ( auto v : values )
		{
		auto e = new PQ_Element(v);
		q.insert(e);
		printf("heap_size: %ld\n", q.size());
		for ( auto v : q )
			printf("time: %.3f\n", v->Time());
		}
	}

	{
	rollbear::prio_queue<8, double, PQ_Element*> q;
	for ( auto r : values )
		{
		auto* e = new PQ_Element(r);
		q.push(r, e);
		}

	while ( ! q.empty() )
		{
		auto e = q.top();
		printf("%f\n", e.first);
		delete e.second;
		q.pop();
		}
	}
	}

void quick_test()
	{
	PriorityQueue q;
	PQ_Element e1(1);
	PQ_Element e2(3);
	PQ_Element e3(2);
	PQ_Element e4(5);
	PQ_Element e5(4);
	q.Add(&e1);
	q.Add(&e2);
	q.Add(&e3);
	q.Add(&e4);
	q.Add(&e5);

	while ( PQ_Element* e = (PQ_Element*)q.Remove() )
		{
		printf("%.0f\n", e->Time());
		}
	printf("\n");

	PriorityQueueVect q3;

	auto* e21 = new PQ_Element(1);
	auto* e22 = new PQ_Element(3);
	auto* e23 = new PQ_Element(2);
	auto* e24 = new PQ_Element(5);
	auto* e25 = new PQ_Element(4);
	q3.Add(e21);
	q3.Add(e22);
	q3.Add(e23);
	q3.Add(e24);
	q3.Add(e25);

	while ( PQ_Element* e = (PQ_Element*)q3.Remove() )
		{
		printf("%.0f\n", e->Time());
		}
	printf("\n");

	rollbear::prio_queue<16, double, PQ_Element*> q4;
	q4.push(e21->Time(), e21);
	q4.push(e22->Time(), e22);
	q4.push(e23->Time(), e23);
	q4.push(e24->Time(), e24);
	q4.push(e25->Time(), e25);

	while ( ! q4.empty() )
		{
		auto e = q4.top();
		q4.pop();
		printf("%.0f\n", e.second->Time());
		}
	}

void benchmark()
	{
	std::vector<int> randoms = generate_randoms(200000);
    std::vector<double> ordered;

    double stock_add, stock_remove;

	struct timeval start;
	struct timeval mid;
	struct timeval end;

		{
		gettimeofday(&start, NULL);

		PriorityQueue q;
		for ( auto r : randoms )
			{
			auto* e = new PQ_Element(r);
			q.Add(e);
			}

		gettimeofday(&mid, NULL);

		while (auto* e = (PQ_Element*)q.Remove() )
			{
			ordered.push_back(e->Time());
			delete e;
			}

		gettimeofday(&end, NULL);
		diagnostics("PQ", start, mid, end);

		stock_add = time_diff(start, mid);
		stock_remove = time_diff(mid, end);
		}

		{
		gettimeofday(&start, NULL);

		StdPQ q;
		for ( auto r : randoms )
			{
			auto* e = new PQ_Element(r);
			q.push(e);
			}

		gettimeofday(&mid, NULL);

		while ( ! q.empty() )
			{
			PQ_Element* e = q.top();
			q.pop();
			delete e;
			}

		gettimeofday(&end, NULL);
		diagnostics("StdPQ", start, mid, end, {}, {}, stock_add, stock_remove);
		}

		{
		std::vector<double> output;
		gettimeofday(&start, NULL);

		absl::btree_set<PQ_Element*, Comparer> q;
		for ( auto r : randoms )
			{
			auto* e = new PQ_Element(r);
			q.insert(e);
			}

		gettimeofday(&mid, NULL);

		while ( ! q.empty() )
			{
			PQ_Element* t = *(q.begin());
//			output.push_back((*q.begin())->Time());
			q.erase(q.begin());
			delete t;
			}

		gettimeofday(&end, NULL);

		diagnostics("absl::btree_set", start, mid, end, {}, {}, stock_add, stock_remove);
		}

		{
		std::vector<double> output;
		gettimeofday(&start, NULL);

		std::set<PQ_Element*, Comparer> q;
		for ( auto r : randoms )
			{
			auto* e = new PQ_Element(r);
			q.emplace(std::move(e));
			}

		gettimeofday(&mid, NULL);

		while ( ! q.empty() )
			{
//			output.push_back((*q.begin())->Time());
			q.erase(q.begin());
			}

		gettimeofday(&end, NULL);

		diagnostics("std::multiset", start, mid, end, {}, {}, stock_add, stock_remove);
		}

/*
  		// This one is *extremely* slow in comparison to the others
		{
		std::vector<double> output;
		gettimeofday(&start, NULL);

		phmap::parallel_flat_hash_set<PQ_Element> q;
		for ( auto r : randoms )
			{
			PQ_Element e(r);
			q.emplace(e);
			}

		gettimeofday(&mid, NULL);

		while ( ! q.empty() )
			{
//			output.push_back((*q.begin()).Time());
			q.erase(q.begin());
			}

		gettimeofday(&end, NULL);

		diagnostics("phmap::flat_hash_set", start, mid, end, {}, {}, stock_add, stock_remove);
		}
*/

		{
		gettimeofday(&start, NULL);

		rollbear::prio_queue<8, double, PQ_Element*> q;
		for ( auto r : randoms )
			{
			auto* e = new PQ_Element(r);
			q.push(r, e);
			}

		gettimeofday(&mid, NULL);

		while ( ! q.empty() )
			{
			auto e = q.top();
			delete e.second;
			q.pop();
			}

		gettimeofday(&end, NULL);
		diagnostics("rollbear-8", start, mid, end, {}, {}, stock_add, stock_remove);
		}

		{
		gettimeofday(&start, NULL);

		rollbear::prio_queue<16, double, PQ_Element*> q;
		for ( auto r : randoms )
			{
			auto* e = new PQ_Element(r);
			q.push(r, e);
			}

		gettimeofday(&mid, NULL);

		while ( ! q.empty() )
			{
			auto e = q.top();
			delete e.second;
			q.pop();
			}

		gettimeofday(&end, NULL);
		diagnostics("rollbear-16", start, mid, end, {}, {}, stock_add, stock_remove);
		}

		{
		gettimeofday(&start, NULL);

		rollbear::prio_queue<32, double, PQ_Element*> q;
		for ( auto r : randoms )
			{
			auto* e = new PQ_Element(r);
			q.push(r, e);
			}

		gettimeofday(&mid, NULL);

		while ( ! q.empty() )
			{
			auto e = q.top();
			delete e.second;
			q.pop();
			}

		gettimeofday(&end, NULL);
		diagnostics("rollbear-32", start, mid, end, {}, {}, stock_add, stock_remove);
		}

		{
		gettimeofday(&start, NULL);

		rollbear::prio_queue<64, double, PQ_Element*> q;
		for ( auto r : randoms )
			{
			auto* e = new PQ_Element(r);
			q.push(r, e);
			}

		gettimeofday(&mid, NULL);

		while ( ! q.empty() )
			{
			auto e = q.top();
			delete e.second;
			q.pop();
			}

		gettimeofday(&end, NULL);
		diagnostics("rollbear-64", start, mid, end, {}, {}, stock_add, stock_remove);
		}
	}

int main(int argc, char** argv)
	{
	benchmark();
//	ordering();
	}
