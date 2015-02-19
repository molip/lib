#include "MeshAnimation.h"

#include "ObjMesh.h"

using namespace Jig;

MeshAnimation::MeshAnimation(const ObjMesh& mesh) : m_mesh(mesh), m_duration(1)
{
	m_root.name = "Body";

	m_root.parts.emplace_back("Left_leg", Vec3(0, 0.6f, 0), Vec3(1, 0, 0));
	m_root.parts.emplace_back("Right_leg", Vec3(0, 0.6f, 0), Vec3(1, 0, 0));

	m_frames[0].angles["Left_leg"] = 0;
	m_frames[0].angles["Right_leg"] = 0;

	m_frames[1].angles["Left_leg"] = 50;
	m_frames[1].angles["Right_leg"] = -50;

	m_frames[2].angles["Left_leg"] = 0;
	m_frames[2].angles["Right_leg"] = 0;

	m_frames[3].angles["Left_leg"] = -50;
	m_frames[3].angles["Right_leg"] = 50;
}

MeshAnimation::~MeshAnimation()
{
}

void MeshAnimation::Draw(float time)
{
	float tickDur = m_duration / (m_frames.rbegin()->first + 1);
	int tick = int(::fmod(time, m_duration) / tickDur);

	const Frame& frame = m_frames.lower_bound(tick)->second;
	m_root.Draw(m_mesh, frame);
}

void MeshAnimation::Part::Draw(const ObjMesh& mesh, const Frame& frame) const
{
	glPushMatrix();

	glTranslatef(pivot.x, pivot.y, pivot.z);
	glRotatef(frame.GetAngle(name), axis.x, axis.y, axis.z);
	glTranslatef(-pivot.x, -pivot.y, -pivot.z);

	mesh.DrawObject(name);

	for (auto& part : parts)
		part.Draw(mesh, frame);

	glPopMatrix();
}

float MeshAnimation::Frame::GetAngle(const std::string& partName) const
{
	auto it = angles.find(partName);
	if (it == angles.end())
		return 0;
	return it->second;
}
