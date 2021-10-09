#pragma once
#include "pch.h"
#include <vector>
#include "Camera.h"
#include "BaseEffect.h"
#include "Texture.h"

class Mesh final
{
public:
	struct Vertex_Input
	{
		Elite::FPoint3 Position;
		Elite::RGBColor Color;
		Elite::FVector2 UV;
		Elite::FVector3 Normal;
		Elite::FVector3 Tangent;

		Vertex_Input(const Elite::FPoint3& position, const Elite::FVector2& uv, const Elite::FVector3& normal, const Elite::FVector3& tangent = {});
		Vertex_Input();
		bool operator==(const Vertex_Input& other) const;
	};

	Mesh(const std::vector<Vertex_Input>& vertices, const std::vector<uint32_t>& indices, ID3D11Device* pDevice, bool canGoTransparant, bool canSwitchCullMode, const std::string& shaderPath,
		const Elite::FMatrix4& worldMatrix = Elite::FMatrix4::Identity());
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(Mesh&&) = delete;
	~Mesh();

	void Update(const Camera* pCamera);


	//Getters
	const Elite::FMatrix4& GetWorldMatrix() const;
	const std::vector<Vertex_Input>& GetVertexBuffer() const;
	const std::vector<uint32_t>& GetIndexBuffer() const;
	ID3D11Buffer* GetVertexBufferGPU() const;
	ID3D11Buffer* GetIndexBufferGPU() const;
	int GetAmountOfIndices() const;

	ID3D11InputLayout* GetInputLayout() const;
	const BaseEffect* GetEffect() const;

	const Texture* GetDiffuse() const;
	const Texture* GetNormal() const;
	const Texture* GetGlossiness() const;
	const Texture* GetSpecular() const;

	const BaseEffect::EffectCullMode& GetCullMode() const;
	bool CanGoTransparant() const;
	bool CanSwitchCullMode() const;

	//Setters
	void SetWorldMatrix(const Elite::FMatrix4& world);
	void SetDiffuseMap(const std::string& path, ID3D11Device* pDevice);
	void SetNormalMap(const std::string& path, ID3D11Device* pDevice);
	void SetGlossinessMap(const std::string& path, ID3D11Device* pDevice);
	void SetSpecularMap(const std::string& path, ID3D11Device* pDevice);
	const BaseEffect::EffectSamplerState& ChangeSamplerState();
	const BaseEffect::EffectCullMode& ChangeCullMode();
	bool ToggleTransparancy();
private:
	//Shared
	Elite::FMatrix4 m_World;
	Elite::FMatrix4 m_WorldViewProj{};
	uint32_t m_AmountIndices = 0;
	Texture* m_pDiffuse = nullptr;
	Texture* m_pNormal = nullptr;
	Texture* m_pGlossiness = nullptr;
	Texture* m_pSpecular = nullptr;

	//Rasterizer
	std::vector<Vertex_Input> m_VertexBuffer;
	std::vector<uint32_t> m_IndexBuffer;

	//DirectX
	ID3D11InputLayout* m_pVertexLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	BaseEffect* m_pEffect = nullptr;

	//Transparancy
	bool m_CanGoTransparant;

	//Culling
	bool m_CanSwitchCullMode;
};

