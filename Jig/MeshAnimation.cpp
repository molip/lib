#include "MeshAnimation.h"

#include "GL.h"
#include "ObjMesh.h"

using namespace Jig;

MeshAnimation::MeshAnimation(const ObjMesh& mesh) : m_mesh(mesh), m_duration(1), m_tickCount(4)
{
	m_root.name = "Body";
	m_root.offsets[0] = Vec3();
	m_root.offsets[1] = Vec3(0, 0.4f, 0);
	m_root.offsets[2] = Vec3();
	m_root.offsets[3] = Vec3(0, 0.4f, 0);

	m_root.parts.emplace_back("Left_leg", Vec3(0, 0.6f, 0), Vec3(1, 0, 0));
	m_root.parts.back().angles[0] = 0;
	m_root.parts.back().angles[1] = 50;
	m_root.parts.back().angles[2] = 0;
	m_root.parts.back().angles[3] = -50;

	m_root.parts.emplace_back("Right_leg", Vec3(0, 0.6f, 0), Vec3(1, 0, 0));
	m_root.parts.back().angles[0] = 0;
	m_root.parts.back().angles[1] = -50;
	m_root.parts.back().angles[2] = 0;
	m_root.parts.back().angles[3] = 50;
}

MeshAnimation::~MeshAnimation()
{
}

void MeshAnimation::Draw(double time)
{
	double tickDur = m_duration / m_tickCount;
	double ticks = ::fmod(time, m_duration) / tickDur;

	m_root.Draw(m_mesh, ticks, m_tickCount);
}

namespace
{
	template <typename T>
	T Lerp(double ticks, int tickCount, std::map<int, T> vals)
	{
		T val = T();
		if (!vals.empty())
		{
			auto b = vals.upper_bound((int)ticks);
			auto a = b;
			if (a != vals.begin())
				--a;

			int aTicks = a->first;
			int bTicks;
			if (b == vals.end())
				b = vals.begin(), bTicks = tickCount;
			else
				bTicks = b->first;

			val = a->second;
			if (a != b)
				val += (b->second - a->second) * (ticks - aTicks) / double(bTicks - aTicks);
		}
		return val;
	}
}

void MeshAnimation::Part::Draw(const ObjMesh& mesh, double ticks, int tickCount) const
{
	if (ticks > 1)
		ticks = ticks;

	double angle = ::Lerp(ticks, tickCount, angles);
	Vec3 offset = ::Lerp(ticks, tickCount, offsets);

	glPushMatrix();

	GL::Translate(offset);

	GL::Translate(pivot);
	GL::Rotate(angle, axis);
	GL::Translate(-pivot);

	mesh.DrawObject(name);

	for (auto& part : parts)
		part.Draw(mesh, ticks, tickCount);

	glPopMatrix();
}
