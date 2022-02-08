// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include <math.h>
#include <stdint.h>
#include <vector>
#include "PQ_Element.h"

class PriorityQueueVect {
public:
	explicit PriorityQueueVect(int initial_size = 16);
	~PriorityQueueVect();

	// Returns the top of queue, or nil if the queue is empty.
	PQ_Element* Top() const
		{
		return heap.empty() ? nullptr : heap.back();
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

	int Size() const	{ return heap.size(); }
	int PeakSize() const	{ return peak_heap_size; }
	uint64_t CumulativeNum() const { return cumulative_num; }

	void Output()
		{
		printf("heap_size: %ld\n", heap.size());
		for ( auto* e : heap )
			printf("offset: %d    time: %.3f\n", e->Offset(), e->Time());
		}

protected:
	bool Resize(int new_size);

	int BubbleUp(int bin);
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

	std::vector<PQ_Element*> heap;
	int peak_heap_size = 0;
	uint64_t cumulative_num = 0;
};
