#include "pch.h"
#include "TransparantEffect.h"
#include <sstream>

TransparantEffect::TransparantEffect(ID3D11Device* pDevice, const std::string& shaderPath)
	: BaseEffect{pDevice, shaderPath}
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::cout << "Variable gDiffuseMap not valid!" << '\n';
}

TransparantEffect::~TransparantEffect()
{
	m_pDiffuseMapVariable->Release();
}

void TransparantEffect::SetDiffuseMap(ID3D11ShaderResourceView* pResource)
{
	if (m_pDiffuseMapVariable->IsValid())
		m_pDiffuseMapVariable->SetResource(pResource);
}

bool TransparantEffect::ToggleTransparancy()
{
	m_IsTransparent = !m_IsTransparent;

	SetTechnique();

	return m_IsTransparent;
}

const std::string TransparantEffect::GetTechniqueName() const
{
	std::stringstream ss{};
	ss << BaseEffect::GetTechniqueName();

	if (!m_IsTransparent) ss << "NoTransparancy";

	return ss.str();
}
