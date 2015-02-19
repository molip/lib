#pragma once

#include "Types.h"

#include <map>
#include <string>
#include <vector>

namespace Jig
{
	class ObjMesh;

	class MeshAnimation
	{
	public:
		MeshAnimation(const ObjMesh& mesh);
		~MeshAnimation();

		void Draw(float time);

	private:
		struct Frame
		{
			std::map<std::string, float> angles; // Part name -> angle.
			float GetAngle(const std::string& partName) const;
		};

		struct Part
		{
			Part() {}
			Part(const std::string& _name, const Vec3& _pivot, const Vec3& _axis) : name(_name), pivot(_pivot), axis(_axis) {}
			void Draw(const ObjMesh& mesh, const Frame& frame) const;

			std::string name;
			Vec3 pivot; // Relative to parent;
			Vec3 axis;
			std::vector<Part> parts;
		};

		std::map<int, Frame> m_frames; // Tick -> frame.
		const ObjMesh& m_mesh;
		Part m_root;
		float m_duration;
	};
}
