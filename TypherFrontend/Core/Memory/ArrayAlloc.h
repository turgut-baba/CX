#ifndef ARRAY_ALLOC_H
#define ARRAY_ALLOC_H

template<typename Type>
class ArrayAlloc {
public:
	std::vector<Type> vec_;

	void push_back(Type mem)
	{
		vec_.push_back(mem);
	}
};

#endif