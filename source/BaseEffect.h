#pragma once
class BaseEffect
{
public:
	BaseEffect(ID3D11Device* pDevice, const std::string& assetFile);
	virtual ~BaseEffect();
	BaseEffect(const BaseEffect&) = delete;
	BaseEffect& operator=(const BaseEffect&) = delete;
	BaseEffect(BaseEffect&&) = delete;
	BaseEffect& operator=(BaseEffect&&) = delete;

	enum class EffectSamplerState
	{
		Point,
		Linear,
		Anisotropic,
		EndOfList
	};

	enum class EffectCullMode
	{
		Back,
		Front,
		None,
		EndOfList
	};

	const ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechnique() const;

	void SetWorldViewProjMatrix(const Elite::FMatrix4& worldViewProj);
	const EffectSamplerState& ChangeSamplerState();
	const EffectCullMode& ChangeCullMode();
	const EffectCullMode& GetCullMode() const;
	void SetCullMode(const EffectCullMode& cullmode);
protected:
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::string& assetFile);

	EffectSamplerState m_Technique{ EffectSamplerState::Point };
	EffectCullMode m_CullMode{ EffectCullMode::Back };
	virtual const std::string GetTechniqueName() const;
	void SetTechnique();

	ID3DX11Effect* m_pEffect = nullptr;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable = nullptr;
};

