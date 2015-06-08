#include "Noise.h"

extern "C" int open_simplex_noise(int64_t seed, struct osn_context **ctx);
extern "C" void open_simplex_noise_free(struct osn_context *ctx);
extern "C" int open_simplex_noise_init_perm(struct osn_context *ctx, int16_t p[], int nelements);
extern "C" double open_simplex_noise2(struct osn_context *ctx, double x, double y);
extern "C" double open_simplex_noise3(struct osn_context *ctx, double x, double y, double z);
extern "C" double open_simplex_noise4(struct osn_context *ctx, double x, double y, double z, double w);

using namespace Jig;

Noise::Noise(int64_t seed)
{
	open_simplex_noise(seed, &m_ctx);
}

Noise::~Noise()
{
	open_simplex_noise_free(m_ctx);
}

double Noise::Get(double x, double y) const
{
	return open_simplex_noise2(m_ctx, x, y);
}

double Noise::Get(double x, double y, double z) const
{
	return open_simplex_noise3(m_ctx, x, y, z);
}

double Noise::Get(double x, double y, double z, double w) const
{
	return open_simplex_noise4(m_ctx, x, y, z, w);
}
