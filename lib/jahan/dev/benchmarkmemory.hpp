// Author: Sean Jahanpour (sean@jahanpour.com)

#ifndef LIB_BENCHMARKMEMORY_H_
#define LIB_BENCHMARKMEMORY_H_

#include <cstdlib>

class MemoryAllocationMetrics
{
public:
	MemoryAllocationMetrics()
	{
	}

	u_int64_t get_current_heap_usage() 
	{
		return current_allocation_;
	}

	u_int64_t get_max_heap_usage() 
	{
		return current_allocation_;
	}

	inline void allocating(u_int64_t size) 
	{
		current_allocation_ += size;
		if(current_allocation_ > max_allocated_) {
			max_allocated_ = current_allocation_;
		}
	}

	inline void freeing(u_int64_t size) 
	{
		current_allocation_ -= size;
	}

private:
	u_int64_t current_allocation_ = 0;
	u_int64_t max_allocated_ = 0;
};

MemoryAllocationMetrics memory_allocation_metrics{};// MemoryAllocationMetrics();

void* operator new(size_t size)
{
	memory_allocation_metrics.allocating(size);
	return malloc(size);
}

void operator delete(void* memory, size_t size)
{
	memory_allocation_metrics.freeing(size);
	free(memory);
}

#endif //LIB_BENCHMARKMEMORY_H_