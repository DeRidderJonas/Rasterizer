#include "pch.h"
#include "MaterialEffect.h"

MaterialEffect::MaterialEffect(ID3D11Device* pDevice, const std::string& shaderPath)
	: BaseEffect{pDevice, shaderPath}
{
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
		std::cout << "m_pMatWorldVariable not valid!" << '\n';

	m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if (!m_pMatViewInverseVariable->IsValid())
		std::cout << "m_pMatViewInverseVariable not valid!" << '\n';

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::cout << "Variable gDiffuseMap not valid!" << '\n';

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
		std::cout << "Variable gNormalMap not valid!" << '\n';

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
		std::cout << "Variable gSpecularMap not valid!" << '\n';

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
		std::cout << "Variable gGlossinessMap not valid!" << '\n';
}

MaterialEffect::~MaterialEffect()
{
	m_pDiffuseMapVariable->Release();
	m_pNormalMapVariable->Release();
	m_pSpecularMapVariable->Release();
	m_pGlossinessMapVariable->Release();
	m_pMatWorldVariable->Release();
	m_pMatViewInverseVariable->Release();
}

void MaterialEffect::SetWorldMatrix(const Elite::FMatrix4& world)
{
	const float* firstElement{ world.data[0] };
	m_pMatWorldVariable->SetMatrix(firstElement);
}

void MaterialEffect::SetViewInverseMatrix(const Elite::FMatrix4& viewInverse)
{
	const float* firstElement{ viewInverse.data[0] };
	m_pMatViewInverseVariable->SetMatrix(firstElement);
}

void MaterialEffect::SetDiffuseMap(ID3D11ShaderResourceView* pResource)
{
	if (m_pDiffuseMapVariable->IsValid())
		m_pDiffuseMapVariable->SetResource(pResource);
}

void MaterialEffect::SetNormalMap(ID3D11ShaderResourceView* pResource)
{
	if (m_pNormalMapVariable->IsValid())
		m_pNormalMapVariable->SetResource(pResource);
}

void MaterialEffect::SetSpecularMap(ID3D11ShaderResourceView* pResource)
{
	if (m_pSpecularMapVariable->IsValid())
		m_pSpecularMapVariable->SetResource(pResource);
}

void MaterialEffect::SetGlossinessMap(ID3D11ShaderResourceView* pResource)
{
	if (m_pGlossinessMapVariable->IsValid())
		m_pGlossinessMapVariable->SetResource(pResource);
}
