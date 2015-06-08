#pragma once

#include <cstdint>

struct osn_context;

namespace Jig
{
	class Noise
	{
	public:
		Noise(int64_t seed);
		~Noise();

		double Get(double x, double y) const;
		double Get(double x, double y, double z) const;
		double Get(double x, double y, double z, double w) const;

	private:
		osn_context* m_ctx;
	};
}
