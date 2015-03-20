#include "Debug.h"

#include <Windows.h>

#define ENABLE_TRACE

namespace Debug
{
	_Tracer Trace;
}

using namespace Debug;

bool _Tracer::Enabled() const
{
#ifdef ENABLE_TRACE
	return true;
#else 
	return false;
#endif
}

void _Tracer::Output(const std::string& str)
{
	::OutputDebugString(str.c_str());
}
