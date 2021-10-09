#pragma once
#include <string>
#include <vector>
#include "EMath.h"
#include "Mesh.h"

class MeshReader final
{
public:
	static void ReadObjFile(const std::string& fileName, std::vector<Mesh::Vertex_Input>& vertexBuffer, std::vector<uint32_t>& indexBuffer);
private:
	struct Face
	{
		int v0, v1, v2;
		int vt0, vt1, vt2;
		int vn0, vn1, vn2;
	};
	static bool IsVertex(const std::string& line, Elite::FPoint3& vertex);
	static bool IsUV(const std::string& line, Elite::FPoint3& uv);
	static bool IsNormal(const std::string& line, Elite::FPoint3& normal);
	static bool IsFace(const std::string& line, Face& face);
};

