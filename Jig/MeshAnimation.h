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

		void Draw(double time);

	private:
		struct Frame
		{
			double angle;
		};

		struct Part
		{
			Part() {}
			Part(const std::string& _name, const Vec3& _pivot, const Vec3& _axis) : name(_name), pivot(_pivot), axis(_axis) {}
			void Draw(const ObjMesh& mesh, double ticks, int tickCount) const;

			std::string name;
			Vec3 pivot;
			Vec3 axis;
			std::vector<Part> parts;
			std::map<int, double> angles; // Tick -> angle.
			std::map<int, Vec3> offsets; // Tick -> offset.
		};

		const ObjMesh& m_mesh;
		Part m_root;
		double m_duration;
		int m_tickCount;
	};
}
