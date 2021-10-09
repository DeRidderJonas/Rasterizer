#include "pch.h"
#include <vld.h>
//#undef main

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"
#include "SceneGraph.h"
#include "MeshReader.h"

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void PrintControls()
{
	std::cout << "---Camera Controls---" << '\n';
	std::cout << "Move:\t\tWASD" << '\n';
	std::cout << "Go up:\t\tE" << '\n';
	std::cout << "Go down:\tQ" << '\n';
	std::cout << "increase FOV:\tLeft-Arrow" << '\n';
	std::cout << "decrease FOV:\tRight-Arrow" << '\n';

	std::cout << "---Key bindings---" << '\n';
	std::cout << "R: swap between DirectX and Software Rasterizer" << '\n';
	std::cout << "F: toggle between texture sampling states (DirectX only)" << '\n';
	std::cout << "T: toggle transparacny on/off (DirectX only)" << '\n';
	std::cout << "C: toggle betwwen cull modes, BackFace - FrontFace - None" << '\n';
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;
	SDL_Window* pWindow = SDL_CreateWindow(
		"Exam Project - **Jonas De Ridder**",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	PrintControls();

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };
	Camera* pCamera = new Camera(float(width), float(height));

	auto pDevice = pRenderer->GetDevice();

	
	std::vector<Mesh::Vertex_Input> vertices{};
	std::vector<uint32_t> indices{};
	Elite::FMatrix4 translation{ Elite::MakeTranslation(Elite::FVector3{0.f,0.f,40.f}) };
	std::cout << "Now loading vehicle.obj, Please wait\n";
	MeshReader::ReadObjFile("Resources/vehicle.obj", vertices, indices);
	Mesh* pVehicle = new Mesh(vertices, indices, pDevice, false, true, "Resources/PosCol3D.fx", translation);
	pVehicle->SetDiffuseMap("Resources/vehicle_diffuse.png", pDevice);
	pVehicle->SetNormalMap("Resources/vehicle_normal.png", pDevice);
	pVehicle->SetGlossinessMap("Resources/vehicle_gloss.png", pDevice);
	pVehicle->SetSpecularMap("Resources/vehicle_specular.png", pDevice);
	SceneGraph::GetInstance()->AddMesh(pVehicle);

	std::vector<Mesh::Vertex_Input> exhaustVertices{};
	std::vector<uint32_t> exhaustIndices{};
	MeshReader::ReadObjFile("Resources/fireFX.obj", exhaustVertices, exhaustIndices);
	Mesh* pExhaust = new Mesh(exhaustVertices, exhaustIndices, pDevice, true, false, "Resources/TransparantShading.fx", translation);
	pExhaust->SetDiffuseMap("Resources/fireFX_diffuse.png", pDevice);
	SceneGraph::GetInstance()->AddMesh(pExhaust);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_R:
					{
						bool usingDirectX = pRenderer->ToggleDirectXRasterizer();
						pCamera->SetHandedNess(usingDirectX);
						if (usingDirectX) std::cout << "Now using DirectX" << '\n';
						else std::cout << "Now using Software Rasterizer" << '\n';
					}
					break;
				case SDL_SCANCODE_F:
					{
						std::vector<Mesh*>& meshes{ SceneGraph::GetInstance()->GetMeshes() };
						BaseEffect::EffectSamplerState newSamplerState{};
						for (Mesh* pMesh : meshes)
							newSamplerState = pMesh->ChangeSamplerState();
						std::cout << "New Samplerstate: ";
						switch (newSamplerState)
						{
						case BaseEffect::EffectSamplerState::Point:
							std::cout << "Point" << '\n';
							break;
						case BaseEffect::EffectSamplerState::Linear:
							std::cout << "Linear" << '\n';
							break;
						case BaseEffect::EffectSamplerState::Anisotropic:
							std::cout << "Anisotropic" << '\n';
							break;
						}
					}
					break;
				case SDL_SCANCODE_C:
					{
						std::vector<Mesh*>& meshes{ SceneGraph::GetInstance()->GetMeshes() };
						BaseEffect::EffectCullMode newCullMode{};
						for (Mesh* pMesh : meshes)
						{
							if (pMesh->CanSwitchCullMode()) newCullMode = pMesh->ChangeCullMode();
						}
						std::cout << "New Culling Mode: ";
						switch (newCullMode)
						{
						case BaseEffect::EffectCullMode::Back:
							std::cout << "Backface" << '\n';
							break;
						case BaseEffect::EffectCullMode::Front:
							std::cout << "Frontface" << '\n';
							break;
						case BaseEffect::EffectCullMode::None:
							std::cout << "No culling" << '\n';
							break;
						}
					}
					break;
				case SDL_SCANCODE_T:
					std::vector<Mesh*>& meshes{ SceneGraph::GetInstance()->GetMeshes() };
					bool isTransparant{};
					for (Mesh* pMesh : meshes)
					{
						if (pMesh->CanGoTransparant()) isTransparant = pMesh->ToggleTransparancy();
					}
					std::cout << "Transparancy: " << (isTransparant ? "On" : "Off") << '\n';
				}
				break;
			}
		}

		//--------- Update ---------
		pCamera->Move(pTimer->GetElapsed());

		//--------- Render ---------
		pRenderer->Render(pCamera);

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}

	}
	pTimer->Stop();

	//Shutdown "framework"
	ShutDown(pWindow);
	delete pCamera;
	delete SceneGraph::GetInstance();

	return 0;
}