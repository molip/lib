#include "MeshAnimation.h"

#include "ObjMesh.h"

using namespace Jig;

MeshAnimation::MeshAnimation(const ObjMesh& mesh) : m_mesh(mesh), m_duration(1), m_tickCount(4)
{
	m_root.name = "Body";

	m_root.parts.emplace_back("Left_leg", Vec3(0, 0.6f, 0), Vec3(1, 0, 0));
	m_root.parts.back().frames[0].angle = 0;
	m_root.parts.back().frames[1].angle = 50;
	m_root.parts.back().frames[2].angle = 0;
	m_root.parts.back().frames[3].angle = -50;

	m_root.parts.emplace_back("Right_leg", Vec3(0, 0.6f, 0), Vec3(1, 0, 0));
	m_root.parts.back().frames[0].angle = 0;
	m_root.parts.back().frames[1].angle = -50;
	m_root.parts.back().frames[2].angle = 0;
	m_root.parts.back().frames[3].angle = 50;
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

	if (!frames.empty())
	{
		auto a = frames.lower_bound((int)ticks);
		if (a == frames.end())
			--a;

		auto b = frames.upper_bound((int)ticks);
		int aTicks = a->first;
		int bTicks;
		if (b == frames.end())
			b = frames.begin(), bTicks = tickCount;
		else
			bTicks = b->first;

		angle = a->second.angle;
		if (a != b)
			angle += (b->second.angle - a->second.angle) * (ticks - aTicks) / (bTicks - aTicks);
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
