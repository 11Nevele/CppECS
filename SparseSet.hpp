#ifndef SPARSE_SET
#define SPARSE_SET

#include <type_traits>
#include <vector>
#include <cassert>
//make type is an unsigned integer
template <class TYPE>
requires std::is_unsigned<TYPE>::value
class SparseSet final
{
private:
	std::vector<TYPE> dense;
	std::vector<TYPE> sparse;
	uint32_t maxValue;
public:
	SparseSet(TYPE maxValue = 10000)
	{
		this->maxValue = maxValue;
		sparse.resize(maxValue + 1, INT_MAX);
	}
	~SparseSet()
	{
		dense.clear();
		sparse.clear();
	}

	//add an element to the set
	void Add(TYPE element)
	{
		assert(element <= maxValue);
		dense.push_back(element);
		sparse[element] = dense.size() - 1;
	}
	bool Contains(TYPE element) const
	{
		return sparse[element] < dense.size() && dense[sparse[element]] == element;
	}
	void Remove(TYPE element)
	{
		assert(Contains(element));
		TYPE lastElement = dense.back();
		dense[sparse[element]] = lastElement;
		sparse[lastElement] = sparse[element];
		sparse[element] = INT_MAX;
		dense.pop_back();
	}

};
#endif