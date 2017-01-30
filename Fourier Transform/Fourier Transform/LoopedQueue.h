#pragma once
#include <assert.h>

template <typename T>
class LoopedList
{
private:
	unsigned int m_capacity;
	unsigned int m_cursor;
	unsigned int m_size;
	T* m_list;
public:
	LoopedList(unsigned int _capacity)
	{
		m_capacity = _capacity;
		m_size = 0;
		m_cursor = 0;
		m_list = new T[m_capacity];
	}
	~LoopedList()
	{
		delete[] m_list;
	}

	T get(unsigned int _index)
	{
		assert(m_size > _index);
		//if (m_cursor > _index)
		//{
			return m_list[_index];
		//}
		//return T();
	}
	T getRelative(int _offset)
	{
		
		return get(((int)m_cursor + _offset) % m_capacity);
	}
	void add(const T& _item)
	{
		m_list[m_cursor] = _item;
		m_cursor++;
		if (m_cursor >= m_capacity)
		{
			m_cursor = 0;
		}
		if (m_size < m_capacity)
		{
			m_size++;
		}
	}
	unsigned int getCapacity()
	{
		return m_capacity;
	}
	unsigned int getSize()
	{
		return m_size;
	}
	unsigned int getCursorPos()
	{
		return m_cursor;
	}
};
