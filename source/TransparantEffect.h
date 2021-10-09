#pragma once
#include "BaseEffect.h"

class TransparantEffect : public BaseEffect
{
public:
	TransparantEffect(ID3D11Device* pDevice, const std::string& shaderPath);
	virtual ~TransparantEffect();

	void SetDiffuseMap(ID3D11ShaderResourceView* pResource);
	bool ToggleTransparancy();
private:
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable = nullptr;
	virtual const std::string GetTechniqueName() const override;
	bool m_IsTransparent = true;
};

