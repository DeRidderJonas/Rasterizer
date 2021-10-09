#include "pch.h"
#include "MeshReader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

void MeshReader::ReadObjFile(const std::string& fileName, std::vector<Mesh::Vertex_Input>& vertexBuffer, std::vector<uint32_t>& indexBuffer)
{
    std::ifstream input{ fileName };
    std::string line{};

    Elite::FPoint3 fpoint3{};
    Face face{};

    std::vector<Elite::FPoint3> posBuffer{};
    std::vector<Elite::FPoint3> uvBuffer{};
    std::vector<Elite::FPoint3> normalBuffer{};

    if (input.is_open())
    {
        while (std::getline(input, line))
        {
            if (IsVertex(line, fpoint3))
            {
                posBuffer.push_back(fpoint3);
            }

            if (IsUV(line, fpoint3))
            {
                uvBuffer.push_back(fpoint3);
            }

            if (IsNormal(line, fpoint3))
            {
                normalBuffer.push_back(fpoint3);
            }

            if (IsFace(line, face))
            {
                Mesh::Vertex_Input v0{ posBuffer[face.v0], Elite::FVector2{Elite::FPoint2{uvBuffer[face.vt0]}}, Elite::FVector3{normalBuffer[face.vn0]} };
                Mesh::Vertex_Input v1{ posBuffer[face.v1], Elite::FVector2{Elite::FPoint2{uvBuffer[face.vt1]}}, Elite::FVector3{normalBuffer[face.vn1]} };
                Mesh::Vertex_Input v2{ posBuffer[face.v2], Elite::FVector2{Elite::FPoint2{uvBuffer[face.vt2]}}, Elite::FVector3{normalBuffer[face.vn2]} };

                //https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
                const Elite::FVector3 edge0 = Elite::FPoint3{ v1.Position } - Elite::FPoint3{ v0.Position};
                const Elite::FVector3 edge1 = Elite::FPoint3{ v2.Position } - Elite::FPoint3{ v0.Position };
                const Elite::FVector2 diffX{ v1.UV.x - v0.UV.x, v2.UV.x - v0.UV.x };
                const Elite::FVector2 diffY{ v1.UV.y - v0.UV.y, v2.UV.y - v0.UV.y };
                float r = 1.f / Elite::Cross(diffX, diffY);

                Elite::FVector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
                v0.Tangent = -Elite::GetNormalized(Elite::Reject(tangent, v0.Normal));
                v1.Tangent = -Elite::GetNormalized(Elite::Reject(tangent, v1.Normal));
                v2.Tangent = -Elite::GetNormalized(Elite::Reject(tangent, v2.Normal));

                //Check if duplicate vertex already exist
                //If not, add it to vertexBuffer
                const int invalid_index{ -1 };
                int i0{ invalid_index }, i1{ invalid_index }, i2{ invalid_index };
                int i{0};
                for (const Mesh::Vertex_Input& v : vertexBuffer)
                {
                    if (v == v0) i0 = i;
                    if (v == v1) i1 = i;
                    if (v == v2) i2 = i;
                    i++;
                }

                if (i0 == invalid_index)
                {
                    vertexBuffer.push_back(v0);
                    i0 = int(vertexBuffer.size()) - 1;
                }

                if (i1 == invalid_index)
                {
                    vertexBuffer.push_back(v1);
                    i1 = int(vertexBuffer.size()) - 1;
                }

                if (i2 == invalid_index)
                {
                    vertexBuffer.push_back(v2);
                    i2 = int(vertexBuffer.size()) - 1;
                }

                indexBuffer.push_back(i0);
                indexBuffer.push_back(i1);
                indexBuffer.push_back(i2);
            }
        }
    }
}

bool MeshReader::IsVertex(const std::string& line, Elite::FPoint3& vertex)
{
    std::stringstream ss{ line };
    std::string string;
    std::getline(ss, string, ' ');
    if (string != "v") return false;

    ss >> vertex.x;
    ss >> vertex.y;
    ss >> vertex.z;

    vertex.z = -vertex.z;

    if (ss.fail()) return false;

    return true;
}

bool MeshReader::IsUV(const std::string& line, Elite::FPoint3& uv)
{
    std::stringstream ss{ line };
    std::string string;
    std::getline(ss, string, ' ');
    if (string != "vt") return false;

    ss >> uv.x;
    ss >> uv.y;
    ss >> uv.z;

    uv.y = 1 - uv.y;

    if (ss.fail()) return false;

    return true;
}

bool MeshReader::IsNormal(const std::string& line, Elite::FPoint3& normal)
{
    std::stringstream ss{ line };
    std::string string;
    std::getline(ss, string, ' ');
    if (string != "vn") return false;

    ss >> normal.x;
    ss >> normal.y;
    ss >> normal.z;

    normal.z = -normal.z;

    if (ss.fail()) return false;

    return true;
}

bool MeshReader::IsFace(const std::string& line, Face& face)
{
    std::stringstream ss{ line };
    std::string string;
    std::getline(ss, string, ' ');
    if (string != "f") return false;

    try
    {
        //Vertex 0
        std::getline(ss, string, '/');
        face.v0 = std::stoi(string);
        std::getline(ss, string, '/');
        face.vt0 = std::stoi(string);
        std::getline(ss, string, ' ');
        face.vn0 = std::stoi(string);

        //Vertex 1
        std::getline(ss, string, '/');
        face.v1 = std::stoi(string);
        std::getline(ss, string, '/');
        face.vt1 = std::stoi(string);
        std::getline(ss, string, ' ');
        face.vn1 = std::stoi(string);

        //Vertex 2
        std::getline(ss, string, '/');
        face.v2 = std::stoi(string);
        std::getline(ss, string, '/');
        face.vt2 = std::stoi(string);
        std::getline(ss, string, ' ');
        face.vn2 = std::stoi(string);
    }
    catch (const std::exception& e)
    {
        std::cout << "Something went wrong when parsing obj face: " << e.what() << '\n';
        return false;
    }

    //Obj starts indexing at 1
    --face.v0;
    --face.v1;
    --face.v2;

    --face.vn0;
    --face.vn1;
    --face.vn2;
    
    --face.vt0;
    --face.vt1;
    --face.vt2;

    if (ss.fail()) return false;

    return true;
}
