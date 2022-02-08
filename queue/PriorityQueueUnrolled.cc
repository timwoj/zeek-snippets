// See the file "COPYING" in the main distribution directory for copyright.

#include <stdio.h>
#include <stdlib.h>

#include "PriorityQueueUnrolled.h"

PriorityQueueUnrolled::PriorityQueueUnrolled(int initial_size)
	{
	max_heap_size = initial_size;
	heap = new PQ_Element*[max_heap_size];
	peak_heap_size = heap_size = cumulative_num = 0;
	}

PriorityQueueUnrolled::~PriorityQueueUnrolled()
	{
	for ( int i = 0; i < heap_size; ++i )
		delete heap[i];

	delete [] heap;
	}

PQ_Element* PriorityQueueUnrolled::Remove()
	{
	if ( heap_size == 0 )
		return nullptr;

	PQ_Element* top = heap[0];

	// Shrink the heap by one so that we can get to the back element
	--heap_size;

	// Swap the back element to the front
	SetElement(0, heap[heap_size]);

	// Move the new front element to a position that makes sense
	BubbleDown(0);

	top->offset = -1; // = not in heap
	return top;
	}

PQ_Element* PriorityQueueUnrolled::Remove(PQ_Element* e)
	{
	if ( e->Offset() < 0 || e->Offset() >= heap_size ||
	     heap[e->Offset()] != e )
		return nullptr;	// not in heap

	e->MinimizeTime();
	BubbleUp(e->Offset());

	PQ_Element* e2 = Remove();

	if ( e != e2 )
		{
		printf("inconsistency in PriorityQueueUnrolled::Remove\n");
		abort();
		}

	return e2;
	}

bool PriorityQueueUnrolled::Add(PQ_Element* e)
	{
	SetElement(heap_size, e);

	BubbleUp(heap_size);

	++cumulative_num;
	++heap_size;

	if ( heap_size > peak_heap_size )
		peak_heap_size = heap_size;

	if ( heap_size >= max_heap_size )
		return Resize(max_heap_size * 2);
	else
		return true;
	}

bool PriorityQueueUnrolled::Resize(int new_size)
	{
	PQ_Element** tmp = new PQ_Element*[new_size];
	for ( int i = 0; i < max_heap_size; ++i )
		tmp[i] = heap[i];

	delete [] heap;
	heap = tmp;

	max_heap_size = new_size;

	return heap != nullptr;
	}

void PriorityQueueUnrolled::BubbleUp(int bin)
	{
//	printf("   BubbleUp: %d\n", bin);
	if ( bin == 0 )
		return;

	int p = bin >> 1;
	if ( heap[p]->Time() > heap[bin]->Time() )
		{
		Swap(p, bin);
		BubbleUp(p);
		}
	}

void PriorityQueueUnrolled::BubbleDown(int bin)
	{
//	printf("   BubbleDown: %d\n", bin);
	int old_bin = -1;
	while ( old_bin != bin )
		{
		int l = bin << 1;
		if ( l >= heap_size )
			break;		// No children.

		old_bin = bin;
		double v = heap[bin]->time;
		int r = l + 1;

		if ( r >= heap_size )
			{ // Just a left child.
			if ( heap[l]->time < v )
				Swap(l, bin);
			}

		else
			{
			double lv = heap[l]->time;
			double rv = heap[r]->time;

			if ( lv < rv )
				{
				if ( lv < v )
					{
					Swap(l, bin);
					bin = l;
					}
				}

			else if ( rv < v )
				{
				Swap(r, bin);
				bin = r;
				}
			}
		}
	}
