// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include <math.h>
#include <stdint.h>
#include "PQ_Element.h"

class PriorityQueue {
public:
	explicit PriorityQueue(int initial_size = 16);
	~PriorityQueue();

	// Returns the top of queue, or nil if the queue is empty.
	PQ_Element* Top() const
		{
		if ( heap_size == 0 )
			return 0;
		else
			return heap[0];
		}

	// Removes (and returns) top of queue.  Returns nil if the queue
	// is empty.
	PQ_Element* Remove();

	// Removes element e.  Returns e, or nullptr if e wasn't in the queue.
	// Note that e will be modified via MinimizeTime().
	PQ_Element* Remove(PQ_Element* e);

	// Add a new element to the queue.  Returns false on failure (not enough
	// memory to add the element), true on success.
	bool Add(PQ_Element* e);

	int Size() const	{ return heap_size; }
	int PeakSize() const	{ return peak_heap_size; }
	uint64_t CumulativeNum() const { return cumulative_num; }

	void Output()
		{
		printf("heap_size: %d   max_size: %d\n", heap_size, max_heap_size);
		for ( int i = 0; i < heap_size; i++)
			printf("offset: %d    time: %.3f\n", heap[i]->Offset(), heap[i]->Time());
		}

protected:
	bool Resize(int new_size);

	void BubbleUp(int bin);
	void BubbleDown(int bin);

	int Parent(int bin) const
		{
		return bin >> 1;
		}

	int LeftChild(int bin) const
		{
		return bin << 1;
		}

	int RightChild(int bin) const
		{
		return LeftChild(bin) + 1;
		}

	void SetElement(int bin, PQ_Element* e)
		{
		heap[bin] = e;
		e->SetOffset(bin);
		}

	void Swap(int bin1, int bin2)
		{
		PQ_Element* t = heap[bin1];
		SetElement(bin1, heap[bin2]);
		SetElement(bin2, t);
		}

	PQ_Element** heap;
	int heap_size;
	int peak_heap_size;
	int max_heap_size;
	uint64_t cumulative_num;
};
