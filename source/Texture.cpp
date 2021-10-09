#include "pch.h"
#include "Texture.h"
#include <SDL_image.h>

Texture::Texture(const std::string& filePath, ID3D11Device* pDevice)
{
	m_pTexture = IMG_Load(filePath.c_str());

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pTexture->w;
	desc.Height = m_pTexture->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_pTexture->pixels;
	initData.SysMemPitch = static_cast<UINT>(m_pTexture->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(m_pTexture->h * m_pTexture->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTextureGPU);
	if (FAILED(hr))
		return;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pTextureGPU, &SRVDesc, &m_pTextureResourceView);
	if (FAILED(hr))
		return;

}

Texture::~Texture()
{
	m_pTextureResourceView->Release();
	m_pTextureGPU->Release();
	SDL_FreeSurface(m_pTexture);
}

ID3D11ShaderResourceView* Texture::GetTextureResourceView() const
{
	return m_pTextureResourceView;
}

const Elite::RGBColor Texture::Sample(const Elite::FVector2& uv) const
{
	if (uv.x < 0 || uv.x > 1.0f || uv.y < 0 || uv.y > 1.f) return Elite::RGBColor{};

	Elite::FVector2 UV{ uv };
	UV.x *= m_pTexture->w;
	UV.y *= m_pTexture->h;

	Uint32 pixelIndex{ Uint32(UV.x) + Uint32(UV.y) * m_pTexture->w };

	Uint8 r{}, g{}, b{};
	Uint32* pixels = (Uint32*)m_pTexture->pixels;
	Uint32 pPixel = pixels[pixelIndex];

	SDL_GetRGB(pPixel, m_pTexture->format, &r, &g, &b);

	return Elite::RGBColor{ r / 255.f, g / 255.f, b / 255.f };
}
