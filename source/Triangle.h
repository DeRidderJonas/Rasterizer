#pragma once
#include "Mesh.h"
#include "EMath.h"
#include <vector>

class Triangle final
{
public:
	struct VertexOut
	{
		Elite::FPoint4 position{};
		Elite::RGBColor color{};
		Elite::FVector2 uv{};
		Elite::FVector3 normal{};
		Elite::FVector3 tangent{};
		Elite::FPoint3 worldPosition{};
	};
	Triangle(const Mesh::Vertex_Input& v0, const Mesh::Vertex_Input& v1, const Mesh::Vertex_Input& v2);
	~Triangle() = default;

	const Elite::FVector3 GetTriangleNormal(const Elite::FMatrix4& world) const;
	const Elite::FPoint3 GetTriangleMiddle(const Elite::FMatrix4& world) const;
	bool Hit(const Elite::FPoint2& screenSpacePixel, float screenWidth, float screenHeight, bool frontFaceCulling, VertexOut& vertex, float& weight0, float& weight1, float& weight2) const;
	void UpdateProjectionSpace(const Elite::FMatrix4& worldViewProj);
	bool IsFrustumCulled(const Camera* pCamera) const;
	void GetBoundingBox(Elite::FPoint2& topLeft, Elite::FPoint2& bottomRight, float screenWidth, float screenHeight) const;
	void Interpolate(const Elite::FMatrix4& world, VertexOut& vertex, float weight0, float weight1, float weight2) const;
private:
	static const size_t m_AmountOfVertices{ 3 };
	Mesh::Vertex_Input m_InputVertices[m_AmountOfVertices];

	Elite::FPoint4 m_ProjectedVertices[m_AmountOfVertices];
};

