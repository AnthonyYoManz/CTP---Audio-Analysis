//Queue-like template with .at() function
//Constrained in the sense that if you push over capacity then earliest element will be removed

#pragma once

#include <vector>

template<typename t>
class ConstrainedQueue
{
private:
	std::vector<t> m_container;
	unsigned int m_capacity;
public:
	ConstrainedQueue(unsigned int _capacity = 100)
	{
		m_capacity = _capacity;
	}

	~ConstrainedQueue() = default;

	t at(unsigned int _index)
	{
		return m_container.at(_index);
	}

	void push(t _input)
	{
		m_container.push_back(_input);
		while (m_container.size() > m_capacity)
		{
			m_container.erase(m_container.begin());
		}
	}

	void setCapacity(unsigned int _capacity)
	{
		m_capacity = _capacity;
		while (m_container.size() > m_capacity)
		{
			m_container.erase(m_container.begin());
		}
	}

	std::vector<t>& getVector()
	{
		return m_container;
	}

	unsigned int getCapacity()
	{
		return m_capacity;
	}

	unsigned int getCurrentSize()
	{
		return m_container.size();
	}
};