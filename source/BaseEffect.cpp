#include "pch.h"
#include "BaseEffect.h"
#include "Converter.h"
#include <sstream>

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::string& assetFile)
	: m_pEffect{ BaseEffect::LoadEffect(pDevice, assetFile) }
{
	SetTechnique();

	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
		std::cout << "m_pMatWorldViewProjVariable not valid!" << '\n';
}

BaseEffect::~BaseEffect()
{
	m_pMatWorldViewProjVariable->Release();
	m_pTechnique->Release();
	m_pEffect->Release();
}

const ID3DX11Effect* BaseEffect::GetEffect() const
{
	return m_pEffect;
}

ID3DX11EffectTechnique* BaseEffect::GetTechnique() const
{
	return m_pTechnique;
}

void BaseEffect::SetWorldViewProjMatrix(const Elite::FMatrix4& worldViewProj)
{
	const float* firstElement{ worldViewProj.data[0] };
	m_pMatWorldViewProjVariable->SetMatrix(firstElement);
}

const BaseEffect::EffectSamplerState& BaseEffect::ChangeSamplerState()
{
	m_Technique = EffectSamplerState((int(m_Technique) + 1) % int(EffectSamplerState::EndOfList));

	SetTechnique();

	return m_Technique;
}

const BaseEffect::EffectCullMode& BaseEffect::ChangeCullMode()
{
	m_CullMode = EffectCullMode((int(m_CullMode) + 1) % int(EffectSamplerState::EndOfList));

	SetTechnique();

	return m_CullMode;
}

const BaseEffect::EffectCullMode& BaseEffect::GetCullMode() const
{
	return m_CullMode;
}

void BaseEffect::SetCullMode(const EffectCullMode& cullmode)
{
	m_CullMode = cullmode;

	SetTechnique();
}

ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::string& assetFile)
{
	HRESULT result = S_OK;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	std::wstring wAssetFile{ Converter::ConvertStringToWString(assetFile) };
	result = D3DX11CompileEffectFromFile(wAssetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			char* pErrors = (char*)pErrorBlob->GetBufferPointer();

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << wAssetFile;
			std::wcout << ss.str() << std::endl;
		}
		return nullptr;
	}

	return pEffect;
}

const std::string BaseEffect::GetTechniqueName() const
{
	std::stringstream ss{};
	switch (m_Technique)
	{
	case BaseEffect::EffectSamplerState::Point:
		ss << "PointTechnique";
		break;
	case BaseEffect::EffectSamplerState::Linear:
		ss << "LinearTechnique";
		break;
	case BaseEffect::EffectSamplerState::Anisotropic:
		ss << "AnisotropicTechnique";
		break;
	}
	switch (m_CullMode)
	{
	case BaseEffect::EffectCullMode::Back:
		ss << "CullBack";
		break;
	case BaseEffect::EffectCullMode::Front:
		ss << "CullFront";
		break;
	case BaseEffect::EffectCullMode::None:
		ss << "CullNone";
		break;
	}
	return ss.str();
}

void BaseEffect::SetTechnique()
{
	std::string techniqueName = GetTechniqueName();
	m_pTechnique = m_pEffect->GetTechniqueByName(techniqueName.data());
	if (!m_pTechnique->IsValid())
		std::cout << "Technique not valid!" << '\n';
}
