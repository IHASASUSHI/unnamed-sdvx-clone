#pragma once
#include <list>

/* 
	Doubly linked list
	wrapper around std::list
*/
template<typename I>
class List : public std::list<I>
{
public:
	using std::list<I>::list;

	I& AddBack(const I& item = I())
	{
		auto it = insert(end(), item);
		return *it;
	}
	I& AddFront(const I& item = I())
	{
		auto it = insert(begin(), item);
		return *it;
	}

	// Sort function with uppercase for consistency
	template<typename Predicate>
	void Sort(Predicate& pred)
	{
		sort(pred);
	}
};