#include "MeshAnimation.h"

#include "ObjMesh.h"

using namespace Jig;

MeshAnimation::MeshAnimation(const ObjMesh& mesh) : m_mesh(mesh), m_duration(1), m_tickCount(4)
{
	m_root.name = "Body";

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

void MeshAnimation::Draw(float time)
{
	float tickDur = m_duration / m_tickCount;
	float ticks = ::fmod(time, m_duration) / tickDur;

	m_root.Draw(m_mesh, ticks, m_tickCount);
}

void MeshAnimation::Part::Draw(const ObjMesh& mesh, float ticks, int tickCount) const
{
	float angle = 0;

	if (!angles.empty())
	{
		auto a = angles.lower_bound((int)ticks);
		if (a == angles.end())
			--a;

		auto b = angles.upper_bound((int)ticks);
		int aTicks = a->first;
		int bTicks;
		if (b == angles.end())
			b = angles.begin(), bTicks = tickCount;
		else
			bTicks = b->first;

		angle = a->second;
		if (a != b)
			angle += (b->second - a->second) * (ticks - aTicks) / (bTicks - aTicks);
	}

	glPushMatrix();

	glTranslatef(pivot.x, pivot.y, pivot.z);
	glRotatef(angle, axis.x, axis.y, axis.z);
	glTranslatef(-pivot.x, -pivot.y, -pivot.z);

	mesh.DrawObject(name);

	for (auto& part : parts)
		part.Draw(mesh, ticks, tickCount);

	glPopMatrix();
}
