#include "pch.h"
#include "Triangle.h"
#include "Converter.h"
#include <iostream>
#include <algorithm>

Triangle::Triangle(const Mesh::Vertex_Input& v0, const Mesh::Vertex_Input& v1, const Mesh::Vertex_Input& v2)
	: m_InputVertices{v0, v1, v2}
	, m_ProjectedVertices{}
{
}

const Elite::FVector3 Triangle::GetTriangleNormal(const Elite::FMatrix4& world) const
{
	Elite::FPoint3 worldSpaceCoords[m_AmountOfVertices]{};
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		worldSpaceCoords[i] = Elite::FPoint3{ world * Elite::FPoint4{ m_InputVertices[i].Position } };
	}

	Elite::FVector3 edgeA{ worldSpaceCoords[1] - worldSpaceCoords[0] };
	Elite::FVector3 edgeB{ worldSpaceCoords[2] - worldSpaceCoords[0] };
	
	return Elite::Cross(edgeA, edgeB);
}

const Elite::FPoint3 Triangle::GetTriangleMiddle(const Elite::FMatrix4& world) const
{
	Elite::FPoint3 worldSpaceCoords[m_AmountOfVertices]{};
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		worldSpaceCoords[i] = Elite::FPoint3{ world * Elite::FPoint4{ m_InputVertices[i].Position } };
	}

	return { (worldSpaceCoords[0].x + worldSpaceCoords[1].x + worldSpaceCoords[2].x) / 3.f,
		(worldSpaceCoords[0].y + worldSpaceCoords[1].y + worldSpaceCoords[2].y) / 3.f,
		(worldSpaceCoords[0].z + worldSpaceCoords[1].z + worldSpaceCoords[2].z) / 3.f };
}

bool Triangle::Hit(const Elite::FPoint2& screenSpacePixel, float screenWidth, float screenHeight, bool frontFaceCulling, VertexOut& vertex, float& weight0, float& weight1, float& weight2) const
{
	Elite::FPoint2 RasterSpaceCoords[m_AmountOfVertices]{};
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		RasterSpaceCoords[i] = Elite::FPoint2{ Converter::NDCtoRasterSpace(Elite::FPoint3{ m_ProjectedVertices[i] }, screenWidth, screenHeight) };
	}

	Elite::FVector2 edgeA{ RasterSpaceCoords[1] - RasterSpaceCoords[0] };
	Elite::FVector2 edgeB{ RasterSpaceCoords[2] - RasterSpaceCoords[1] };
	Elite::FVector2 edgeC{ RasterSpaceCoords[0] - RasterSpaceCoords[2] };
	if (frontFaceCulling)
	{
		edgeA *= -1;
		edgeB *= -1;
		edgeC *= -1;
	}

	//Inside-outside test
	Elite::FVector2 pointToSideA{ screenSpacePixel - RasterSpaceCoords[0] };
	weight2 = Elite::Cross(edgeA, pointToSideA);
	if (weight2 > 0) return false;

	Elite::FVector2 pointToSideB{ screenSpacePixel - RasterSpaceCoords[1] };
	weight0 = Elite::Cross(edgeB, pointToSideB);
	if (weight0 > 0) return false;

	Elite::FVector2 pointToSideC{ screenSpacePixel - RasterSpaceCoords[2] };
	weight1 = Elite::Cross(edgeC, pointToSideC);
	if (weight1 > 0) return false;

	float areaParallelogram{ Elite::Cross(RasterSpaceCoords[0] - RasterSpaceCoords[1], RasterSpaceCoords[0] - RasterSpaceCoords[2]) };
	weight0 /= areaParallelogram;
	weight1 /= areaParallelogram;
	weight2 /= areaParallelogram;

	//Interpolate values
	vertex.position.x = screenSpacePixel.x;
	vertex.position.y = screenSpacePixel.y;
	vertex.position.z = 1 / ((1 / m_ProjectedVertices[0].z) * weight0 + (1 / m_ProjectedVertices[1].z) * weight1 + (1 / m_ProjectedVertices[2].z) * weight2);
	vertex.position.w = 1 / ((1 / m_ProjectedVertices[0].w) * weight0 + (1 / m_ProjectedVertices[1].w) * weight1 + (1 / m_ProjectedVertices[2].w) * weight2);

	return true;
}

void Triangle::UpdateProjectionSpace(const Elite::FMatrix4& worldViewProj)
{
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		m_ProjectedVertices[i] = worldViewProj * Elite::FPoint4{ m_InputVertices[i].Position };
		m_ProjectedVertices[i].x /= m_ProjectedVertices[i].w;
		m_ProjectedVertices[i].y /= m_ProjectedVertices[i].w;
		m_ProjectedVertices[i].z /= m_ProjectedVertices[i].w;
	}
}

bool Triangle::IsFrustumCulled(const Camera* pCamera) const
{
	//x y
	//z in world space = w
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		float x{ m_ProjectedVertices[i].x }, y{ m_ProjectedVertices[i].y }, w{ m_ProjectedVertices[i].w };
		if (x < -1 || x > 1 || y < -1 || y > 1 || pCamera->FrustumCull(w)) return true;
	}

	return false;
}

void Triangle::GetBoundingBox(Elite::FPoint2& topLeft, Elite::FPoint2& bottomRight, float screenWidth, float screenHeight) const
{
	Elite::FPoint2 RasterSpaceCoords[m_AmountOfVertices]{};
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		RasterSpaceCoords[i] = Elite::FPoint2{ Converter::NDCtoRasterSpace(Elite::FPoint3{ m_ProjectedVertices[i] }, screenWidth, screenHeight) };
	}

	float minX{ std::min(std::min(RasterSpaceCoords[0].x, RasterSpaceCoords[1].x), RasterSpaceCoords[2].x) };
	float maxX{ std::max(std::max(RasterSpaceCoords[0].x, RasterSpaceCoords[1].x), RasterSpaceCoords[2].x) };
	float minY{ std::min(std::min(RasterSpaceCoords[0].y, RasterSpaceCoords[1].y), RasterSpaceCoords[2].y) };
	float maxY{ std::max(std::max(RasterSpaceCoords[0].y, RasterSpaceCoords[1].y), RasterSpaceCoords[2].y) };

	minX = Elite::Clamp(minX, 0.f, screenWidth - 1);
	maxX = Elite::Clamp(maxX, 0.f, screenWidth - 1);
	minY = Elite::Clamp(minY, 0.f, screenHeight - 1);
	maxY = Elite::Clamp(maxY, 0.f, screenHeight - 1);

	topLeft.x = minX;
	topLeft.y = minY;

	bottomRight.x = maxX;
	bottomRight.y = maxY;

}

void Triangle::Interpolate(const Elite::FMatrix4& world, VertexOut& vertex, float weight0, float weight1, float weight2) const
{
	//World position
	//In vector because there's no float * FPoint operation
	Elite::FVector3 worldPositions[m_AmountOfVertices]{};
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		worldPositions[i] = Elite::FVector3{ Elite::FPoint3{ world * Elite::FPoint4{m_InputVertices[i].Position} } };
	}
	vertex.worldPosition = Elite::FPoint3{ weight0 * worldPositions[0] + weight1 * worldPositions[1] + weight2 * worldPositions[2] };

	//UV
	vertex.uv = (m_InputVertices[0].UV / m_ProjectedVertices[0].w) * weight0 + 
		(m_InputVertices[1].UV / m_ProjectedVertices[1].w) * weight1 + 
		(m_InputVertices[2].UV / m_ProjectedVertices[2].w) * weight2;
	vertex.uv *= vertex.position.w;

	//Preparation for normal and tangent
	Elite::FVector3 worldNormals[m_AmountOfVertices]{};
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		worldNormals[i] = Elite::FVector3{ world * Elite::FVector4{m_InputVertices[i].Normal} };
	}
	Elite::FVector3 worldTangents[m_AmountOfVertices]{};
	for (size_t i = 0; i < m_AmountOfVertices; i++)
	{
		worldTangents[i] = Elite::FVector3{ world * Elite::FVector4{m_InputVertices[i].Tangent} };
	}

	//Normal
	vertex.normal = (worldNormals[0] / m_ProjectedVertices[0].w) * weight0 + 
		(worldNormals[1] / m_ProjectedVertices[1].w) * weight1 + 
		(worldNormals[2] / m_ProjectedVertices[2].w) * weight2;
	vertex.normal *= vertex.position.w;
	Elite::Normalize(vertex.normal);
	//Tangent
	vertex.tangent = (worldTangents[0] / m_ProjectedVertices[0].w) * weight0 + 
		(worldTangents[1] / m_ProjectedVertices[1].w) * weight1 + 
		(worldTangents[2] / m_ProjectedVertices[2].w) * weight2;
	vertex.tangent *= vertex.position.w;
	Elite::Normalize(vertex.tangent);

	//Color
	Elite::RGBColor color{ (m_InputVertices[0].Color / m_ProjectedVertices[0].w) * weight0 + 
		(m_InputVertices[1].Color / m_ProjectedVertices[1].w) * weight1 + 
		(m_InputVertices[2].Color / m_ProjectedVertices[2].w) * weight2 };
	color *= vertex.position.w;
	vertex.color = color;
}
