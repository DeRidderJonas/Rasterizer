#pragma once
#include <vector>
#include "Mesh.h"
class SceneGraph
{
public:
	static SceneGraph* GetInstance();
	SceneGraph(const SceneGraph&) = delete;
	SceneGraph& operator=(const SceneGraph&) = delete;
	SceneGraph(SceneGraph&&) = delete;
	SceneGraph& operator=(SceneGraph&&) = delete;
	~SceneGraph();

	void AddMesh(Mesh* pMesh);
	std::vector<Mesh*>& GetMeshes();
private:
	static SceneGraph* m_Instance;
	SceneGraph() {};

	std::vector<Mesh*> m_Meshes{};
};

