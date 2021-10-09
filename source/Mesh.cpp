#include "pch.h"
#include "Mesh.h"
#include "TransparantEffect.h"
#include "MaterialEffect.h"

Mesh::Vertex_Input::Vertex_Input(const Elite::FPoint3& position, const Elite::FVector2& uv, const Elite::FVector3& normal, const Elite::FVector3& tangent)
	: Position{position}
	, Color{}
	, UV{uv}
	, Normal{normal}
	, Tangent{tangent}
{
}

Mesh::Vertex_Input::Vertex_Input()
	: Position{}
	, Color{}
	, UV{}
	, Normal{}
	, Tangent{}
{
}

bool Mesh::Vertex_Input::operator==(const Vertex_Input& other) const
{
	return Position == other.Position && UV == other.UV && Normal == other.Normal;
}

Mesh::Mesh(const std::vector<Vertex_Input>& vertices, const std::vector<uint32_t>& indices, ID3D11Device* pDevice, bool canGoTransparant, bool canSwitchCullMode
		, const std::string& shaderPath, const Elite::FMatrix4& worldMatrix)
	: m_World{worldMatrix}
	, m_CanGoTransparant{canGoTransparant}
	, m_CanSwitchCullMode{canSwitchCullMode}
	, m_VertexBuffer{vertices}
	, m_IndexBuffer{indices}
{
	if (canGoTransparant) m_pEffect = new TransparantEffect(pDevice, shaderPath);
	else m_pEffect = new MaterialEffect(pDevice, shaderPath);

	if (!canSwitchCullMode) m_pEffect->SetCullMode(BaseEffect::EffectCullMode::None);

	//Create Vertex Layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	//Offset is 12 = 3(x,y,z) * 4(sizeof(float)) = Position
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	//Offset: 3(x,y,z) * 4(sizeof(float)) + 3(r,g,b) * 4(sizeof(float)) = 24
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	//Offset: 3(x,y,z) * 4(sizeof(float)) + 3(r,g,b) * 4(sizeof(float)) + 2(x,y) * 4(sizeof(float))= 32
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	//Offset: 3(x,y,z) * 4(sizeof(float)) + 3(r,g,b) * 4(sizeof(float)) + 2(x,y) * 4(sizeof(float)) + 3(x,y,z) * 4(sizeof(float))= 44
	vertexDesc[4].AlignedByteOffset = 44;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create the input layout
	D3DX11_PASS_DESC passDesc{};
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pVertexLayout
	);
	if (FAILED(result))
		return;

	//Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(Vertex_Input) * (uint32_t)vertices.size();
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData{ 0 };
	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bufferDesc, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	//Create index buffer (reuses Buffer Description and initData from VertexBuffer)
	m_AmountIndices = (uint32_t)indices.size();
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bufferDesc, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;

	for (size_t i = 0; i < m_VertexBuffer.size(); i++)
	{
		m_VertexBuffer[i].Tangent *= -1;
		m_VertexBuffer[i].Position.z *= -1;
		m_VertexBuffer[i].Normal.z *= -1;
	}
}

Mesh::~Mesh()
{
	m_pIndexBuffer->Release();
	m_pVertexBuffer->Release();
	m_pVertexLayout->Release();

	delete m_pEffect;
	delete m_pDiffuse;
	delete m_pNormal;
	delete m_pGlossiness;
	delete m_pSpecular;
}

void Mesh::Update(const Camera* pCamera)
{
	m_WorldViewProj = pCamera->GetProjection() * pCamera->GetView() * m_World;

	m_pEffect->SetWorldViewProjMatrix(m_WorldViewProj);
	if (!m_CanGoTransparant)
	{
		MaterialEffect* pMaterialEffect = reinterpret_cast<MaterialEffect*>(m_pEffect);
		pMaterialEffect->SetWorldMatrix(m_World);
		pMaterialEffect->SetViewInverseMatrix(pCamera->GetONB());
	}
}

const Elite::FMatrix4& Mesh::GetWorldMatrix() const
{
	return m_World;
}

const std::vector<Mesh::Vertex_Input>& Mesh::GetVertexBuffer() const
{
	return m_VertexBuffer;
}

const std::vector<uint32_t>& Mesh::GetIndexBuffer() const
{
	return m_IndexBuffer;
}

ID3D11Buffer* Mesh::GetVertexBufferGPU() const
{
	return m_pVertexBuffer;
}

ID3D11Buffer* Mesh::GetIndexBufferGPU() const
{
	return m_pIndexBuffer;
}

int Mesh::GetAmountOfIndices() const
{
	return m_AmountIndices;
}

ID3D11InputLayout* Mesh::GetInputLayout() const
{
	return m_pVertexLayout;
}

const BaseEffect* Mesh::GetEffect() const
{
	return m_pEffect;
}

const Texture* Mesh::GetDiffuse() const
{
	return m_pDiffuse;
}

const Texture* Mesh::GetNormal() const
{
	return m_pNormal;
}

const Texture* Mesh::GetGlossiness() const
{
	return m_pGlossiness;
}

const Texture* Mesh::GetSpecular() const
{
	return m_pSpecular;
}

const BaseEffect::EffectCullMode& Mesh::GetCullMode() const
{
	return m_pEffect->GetCullMode();
}

bool Mesh::CanGoTransparant() const
{
	return m_CanGoTransparant;
}

bool Mesh::CanSwitchCullMode() const
{
	return m_CanSwitchCullMode;
}

void Mesh::SetWorldMatrix(const Elite::FMatrix4& world)
{
	m_World = world;
}

void Mesh::SetDiffuseMap(const std::string& path, ID3D11Device* pDevice)
{
	if (m_pDiffuse) delete m_pDiffuse;
	m_pDiffuse = new Texture(path, pDevice);

	if (m_CanGoTransparant)
	{
		TransparantEffect* pEffect = reinterpret_cast<TransparantEffect*>(m_pEffect);
		pEffect->SetDiffuseMap(m_pDiffuse->GetTextureResourceView());
	}
	else
	{
		MaterialEffect* pEffect = reinterpret_cast<MaterialEffect*>(m_pEffect);
		pEffect->SetDiffuseMap(m_pDiffuse->GetTextureResourceView());
	}
}

void Mesh::SetNormalMap(const std::string& path, ID3D11Device* pDevice)
{
	if (m_pNormal) delete m_pNormal;
	m_pNormal = new Texture(path, pDevice);

	if (!m_CanGoTransparant)
	{
		MaterialEffect* pEffect = reinterpret_cast<MaterialEffect*>(m_pEffect);
		pEffect->SetNormalMap(m_pNormal->GetTextureResourceView());
	}
}

void Mesh::SetGlossinessMap(const std::string& path, ID3D11Device* pDevice)
{
	if (m_pGlossiness) delete m_pGlossiness;
	m_pGlossiness = new Texture(path, pDevice);

	if (!m_CanGoTransparant)
	{
		MaterialEffect* pEffect = reinterpret_cast<MaterialEffect*>(m_pEffect);
		pEffect->SetGlossinessMap(m_pGlossiness->GetTextureResourceView());
	}
}

void Mesh::SetSpecularMap(const std::string& path, ID3D11Device* pDevice)
{
	if (m_pSpecular) delete m_pSpecular;
	m_pSpecular = new Texture(path, pDevice);

	if (!m_CanGoTransparant)
	{
		MaterialEffect* pEffect = reinterpret_cast<MaterialEffect*>(m_pEffect);
		pEffect->SetSpecularMap(m_pSpecular->GetTextureResourceView());
	}
}

const BaseEffect::EffectSamplerState& Mesh::ChangeSamplerState()
{
	return m_pEffect->ChangeSamplerState();
}

const BaseEffect::EffectCullMode& Mesh::ChangeCullMode()
{
	return m_pEffect->ChangeCullMode();
}

bool Mesh::ToggleTransparancy()
{
	if (!m_CanGoTransparant) return false;

	TransparantEffect* pEffect = reinterpret_cast<TransparantEffect*>(m_pEffect);
	return pEffect->ToggleTransparancy();
}
