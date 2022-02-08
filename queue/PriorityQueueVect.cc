// See the file "COPYING" in the main distribution directory for copyright.

#include <stdio.h>
#include <stdlib.h>

#include "PriorityQueueVect.h"

int sorts = 0;

PriorityQueueVect::PriorityQueueVect(int initial_size)
	{
	heap.reserve(initial_size);
	}

PriorityQueueVect::~PriorityQueueVect()
	{
	for ( auto* e : heap )
		delete e;
	printf("%d\n", sorts);
	}

PQ_Element* PriorityQueueVect::Remove()
	{
	if ( heap.empty() )
		return nullptr;

	PQ_Element* e = heap.back();
	heap.pop_back();
	e->SetOffset(-1);	// = not in heap

	return e;
	}

PQ_Element* PriorityQueueVect::Remove(PQ_Element* e)
	{
	if ( e->Offset() < 0 || e->Offset() >= heap.size() || heap[e->Offset()] != e )
		return nullptr;	// not in heap

	e->MinimizeTime();

	auto it = std::find(heap.begin(), heap.end(), e);
	if ( it != heap.end() )
		{
		heap.erase(it);
		}
	else
		{
		printf("inconsistency in PriorityQueueVect::Remove\n");
		abort();
		}

	return e;
	}

bool PriorityQueueVect::Add(PQ_Element* e)
	{
	if ( heap.empty() )
		{
		heap.push_back(e);
		e->SetOffset(0);
		}
	else
		{
		auto it = std::lower_bound(heap.begin(), heap.end(), e,
			[](const PQ_Element* l, const PQ_Element* r) { return l->Time() > r->Time(); });

		heap.insert(it, e);
		e->SetOffset(1);
		}

//	while ( BubbleUp(heap.size()-1) != 0 );

	++cumulative_num;

	if ( heap.size() > peak_heap_size )
		peak_heap_size = heap.size();

	return true;
	}

int PriorityQueueVect::BubbleUp(int bin)
	{
//	printf("   BubbleUp: %d\n", bin);
	if ( bin == 0 )
		return 0;

	int p = Parent(bin);
	if ( heap[p]->Time() < heap[bin]->Time() )
		{
		Swap(p, bin);
		return BubbleUp(p) + 1;
		}

	return 0;
	}


//	std::vector<int> values = { 9, 4, 8, 1, 6, 7, 2, 5, 10, 3 };
