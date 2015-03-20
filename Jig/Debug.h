#pragma once

#include <sstream>
#include <string>

namespace Debug
{
	class _Tracer;

	extern _Tracer Trace;

	class _Tracer
	{
	public:
		template <typename T> 
		_Tracer& operator << (T val)
		{
			if (Enabled())
			{
				std::ostringstream stream;
				stream << val;
				Output(stream.str());
			}
			return *this;
		}

		_Tracer& operator << (std::ostream& (*pf)(std::ostream&)) // For std::endl.
		{
			if (Enabled())
			{
				std::ostringstream stream;
				stream << pf;
				Output(stream.str());
			}
			return *this;
		}
		bool Enabled() const;
		void Output(const std::string& str);
	};
}