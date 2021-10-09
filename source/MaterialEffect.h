#pragma once
#include "BaseEffect.h"

class MaterialEffect : public BaseEffect
{
public:
	MaterialEffect(ID3D11Device* pDevice, const std::string& shaderPath);
	virtual ~MaterialEffect();

	void SetWorldMatrix(const Elite::FMatrix4& world);
	void SetViewInverseMatrix(const Elite::FMatrix4& viewInverse);
	void SetDiffuseMap(ID3D11ShaderResourceView* pResource);
	void SetNormalMap(ID3D11ShaderResourceView* pResource);
	void SetSpecularMap(ID3D11ShaderResourceView* pResource);
	void SetGlossinessMap(ID3D11ShaderResourceView* pResource);
private:
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable = nullptr;
	ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable = nullptr;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable = nullptr;
};

