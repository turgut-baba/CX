#ifndef ARRAY_ALLOC_H
#define ARRAY_ALLOC_H

#include "Memory/SlabAlloc.h"
#include "Memory/BumpPtrAlloc.h"


template<typename Type>
class ArrayAlloc {
public:
	std::vector<Type> vec_;

	void push_back(Type mem)
	{
		vec_.push_back(mem);
	}

	Type operator[](size_t i) {
		return vec_[i];
	}

	bool empty() {
		return vec_.empty();
	}

	size_t size() {
		return vec_.size();
	}
};




#endif