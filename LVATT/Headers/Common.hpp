#pragma once
#include <ctype.h>

template<class type>
struct ArrayWrapper
{
	type* data = nullptr;	/* main array */
	size_t size = 0;		/* array size */

	size_t iterator = 0;	/* iterator, something that can used to just iterate over the array */

	ArrayWrapper(const size_t& arraySize)
	{
		size = arraySize;
		data = new type[arraySize]();
	}

	ArrayWrapper(){}

	void Delete()
	{
		delete[] data;
	}

	type& operator[](const int& pos)
	{
		return data[pos];
	}
};