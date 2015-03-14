#include "ObjMesh.h"

#include "GL.h"

#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>

using namespace Jig;

ObjMesh::ObjMesh(const char* obj_file)
{
	LoadMesh(obj_file);
}

ObjMesh::ObjMesh()
{
}

ObjMesh::~ObjMesh()
{
}

void ObjMesh::LoadMesh(const string& obj_file)
{
	char token[100], buf[100], v[5][100];	

	int	n_vertex, n_texture, n_normal;
	size_t cur_tex = 0;

	FILE *scene = nullptr;
	errno_t err = ::fopen_s(&scene, obj_file.c_str(), "r");
	s_file = obj_file;

	if (!scene)
		throw;

    std::string strPath;
    size_t n = obj_file.find_last_of('/');
    if (n != std::string::npos)
        strPath = obj_file.substr(0, n + 1);

	cout<<endl<<obj_file<<endl;

	while(!feof(scene))
	{
		token[0] = 0;
		fscanf_s(scene, "%s", token, sizeof(token));

		if (!strcmp(token, "o"))
		{
			fscanf_s(scene, "%s", buf, sizeof(buf));
			objectMap[buf] = objects.size();
			objects.emplace_back();
		}
		else if (!strcmp(token, "g"))
		{
			fscanf_s(scene, "%s", buf, sizeof(buf));
		}

		else if (!strcmp(token,"mtllib"))
		{
			char mat_file[50];
			fscanf_s(scene, "%s", mat_file, sizeof(mat_file));
			LoadTex(strPath + string(mat_file));
		}

		else if (!strcmp(token,"usemtl"))
		{
			fscanf_s(scene, "%s", buf, sizeof(buf));
			cur_tex = matMap[s_file+string("_")+string(buf)];
		}

		else if (!strcmp(token,"v"))
		{
			Vec3 vec;
			fscanf_s(scene, "%lf %lf %lf", &vec.x, &vec.y, &vec.z);
			vList.push_back(vec);
		}

		else if (!strcmp(token,"vn"))
		{
			Vec3 vec;
			fscanf_s(scene, "%lf %lf %lf", &vec.x, &vec.y, &vec.z);
			nList.push_back(vec);
		}
		else if (!strcmp(token,"vt"))
		{
			Vec2 vec;
			fscanf_s(scene, "%lf %lf %lf", &vec.x, &vec.y);
			tList.push_back(vec);
		}

		else if (!strcmp(token,"f"))
		{
			for (int i=0;i<3;i++)
			{
				fscanf_s(scene, "%s", v[i], sizeof(v[i]));
			}

			Vertex	tmp_vertex[3];		// for faceList structure

			for (int i=0;i<3;i++)		// for each vertex of this face
			{
				char str[20], ch;
				int base,offset;
				base = offset = 0;

				// calculate vertex-list index
				while( (ch=v[i][base+offset]) != '/' && (ch=v[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				n_vertex = atoi(str);
				base += (ch == '\0')? offset : offset+1;
				offset = 0;

				// calculate texture-list index
				while( (ch=v[i][base+offset]) != '/' && (ch=v[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				n_texture = atoi(str);
				base += (ch == '\0')? offset : offset+1;
				offset = 0;

				// calculate normal-list index
				while( (ch=v[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				n_normal = atoi(str);

				// obj indices are 1-based.
				tmp_vertex[i].v = n_vertex - 1;
				tmp_vertex[i].t = n_texture - 1;
				tmp_vertex[i].n = n_normal - 1;
				tmp_vertex[i].m = (int)cur_tex;
			}

			objects.back().faceList.push_back(FACE(tmp_vertex[0], tmp_vertex[1], tmp_vertex[2]));
		}

		else if (!strcmp(token,"#"))
			fgets(buf,100,scene);
	}

	if (scene) fclose(scene);
}

void ObjMesh::LoadTex(const string& tex_file)
{
	char	token[100], buf[100];
	float	r,g,b;

	FILE* texture = nullptr;
	::fopen_s(&texture, tex_file.c_str(), "r");

	if (!texture)
	{
		cout << "Can't open material file \"" << tex_file << "\"!" << endl;
		return;
	}

	cout<<tex_file<<endl;

	while(!feof(texture))
	{
		token[0] = NULL;
		fscanf_s(texture, "%s", token, sizeof(token));

		if (!strcmp(token, "newmtl"))
		{
			fscanf_s(texture, "%s", buf, sizeof(buf));
			matMap[s_file + string("_") + string(buf)] = matList.size();
			matList.emplace_back();
		}

		else if (!strcmp(token,"Ka"))
		{
			fscanf_s(texture, "%f %f %f", &r, &g, &b);
			matList.back().ambient = Colour(r, g, b);
		}

		else if (!strcmp(token,"Kd"))
		{
			fscanf_s(texture, "%f %f %f", &r, &g, &b);
			matList.back().diffuse = Colour(r, g, b);
		}

		else if (!strcmp(token,"Ks"))
		{
			fscanf_s(texture, "%f %f %f", &r, &g, &b);
			matList.back().specular = Colour(r, g, b);
		}

		else if (!strcmp(token,"Ns"))
		{
			fscanf_s(texture, "%f", &r);
			matList.back().shininess = (int)r;
		}

		else if (!strcmp(token,"#"))
			fgets(buf,100,texture);
	}

	if (texture) 
		fclose(texture);
}

void ObjMesh::Draw() const
{
	for (auto& obj : objects)
		DrawObject(obj);
}

void ObjMesh::DrawObject(const std::string& name) const
{
	auto it = objectMap.find(name);
	if (it == objectMap.end())
		throw;
	
	DrawObject(objects[it->second]);
}

void ObjMesh::DrawObject(const Object& obj) const
{
    glBegin(GL_TRIANGLES);
	for (auto& face : obj.faceList)
		for (int j = 0; j < 3; ++j)
		{
			const Vertex& vertex = face[j];

			matList[vertex.m].Apply();

			GL::Normal(nList[vertex.n]);
			GL::Vertex(vList[vertex.v]);
	   }
	glEnd();
}

void ObjMesh::Scale(double s)
{
	for (size_t i = 0; i < vList.size(); ++i)
		vList[i] *= s;
}

double ObjMesh::GetHorzRadius() const
{
    double rMax = 0.0;
	for (size_t i = 1; i < vList.size(); ++i) // skip first (default value)
	{
		double x = vList[i].x, z = vList[i].z;
		double r = ::sqrt(x * x + z * z);
		rMax = std::max(r, rMax);
	}
    return rMax;
}

//void ObjMesh::GetBBox()
//{
//	for (size_t i = 0; i < vList.size(); ++i)
//        for (int j = 0; j < 3; ++j)
//            vList[i].ptr[j] *= s;
//}
