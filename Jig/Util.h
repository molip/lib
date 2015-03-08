#pragma once

namespace Util
{
	template <typename T>
	class ReverseAdapter
	{
	public:
		ReverseAdapter(const T& t) : m_t(t) {}

		typename T::const_reverse_iterator begin() const { return m_t.rbegin(); }
		typename T::const_reverse_iterator end() const { return m_t.rend(); }
	private:
		const T& m_t;
	};

	template <typename T>
	ReverseAdapter<T> Reverse(const T& t) { return ReverseAdapter<T>(t); }
}