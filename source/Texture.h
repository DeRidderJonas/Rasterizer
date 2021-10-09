#pragma once
#include <string>
#include <SDL_surface.h>
#include "EMath.h"
#include "ERGBColor.h"

class Texture final
{
public:
	Texture(const std::string& filePath, ID3D11Device* pDevice);
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture(Texture&&) = delete;
	Texture& operator=(Texture&&) = delete;
	~Texture();

	ID3D11ShaderResourceView* GetTextureResourceView() const;
	const Elite::RGBColor Sample(const Elite::FVector2& uv) const;
private:
	ID3D11Texture2D* m_pTextureGPU = nullptr;
	SDL_Surface* m_pTexture = nullptr;
	ID3D11ShaderResourceView* m_pTextureResourceView = nullptr;
};

