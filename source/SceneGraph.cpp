#include "pch.h"
#include "SceneGraph.h"

SceneGraph* SceneGraph::m_Instance{ nullptr };

SceneGraph* SceneGraph::GetInstance()
{
    if (m_Instance == nullptr) m_Instance = new SceneGraph();

    return m_Instance;
}

SceneGraph::~SceneGraph()
{
    for (Mesh* pMesh : m_Meshes) delete pMesh;
}

void SceneGraph::AddMesh(Mesh* pMesh)
{
    m_Meshes.push_back(pMesh);
}

std::vector<Mesh*>& SceneGraph::GetMeshes()
{
    return m_Meshes;
}
