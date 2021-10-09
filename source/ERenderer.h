/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>
#include "Mesh.h"
#include "Triangle.h"

struct SDL_Window;
struct SDL_Surface;

namespace Elite
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Camera* pCamera);
		bool ToggleDirectXRasterizer();
		ID3D11Device* GetDevice();

	private:
		void RenderMesh(Mesh* pMesh, const Camera* pCamera);
		Elite::RGBColor PixelShade(const Mesh* pMesh, const Triangle::VertexOut& vertex, const Elite::FVector3& viewDirection) const;

		SDL_Window* m_pWindow;
		uint32_t m_Width;
		uint32_t m_Height;

		bool m_useDirectX = true;

		//Rasterizer
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;
		float* m_pDepthBuffer = nullptr;

		//DirectX
		bool m_IsInitialized;

		ID3D11Device* m_pDevice = nullptr;
		ID3D11DeviceContext* m_pDeviceContext = nullptr;

		IDXGIFactory* m_pDXGIFactory = nullptr;
		IDXGISwapChain* m_pSwapChain = nullptr;
		ID3D11Texture2D* m_pRenderTargetBuffer = nullptr;
		ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

		ID3D11Texture2D* m_pDepthStencilBuffer = nullptr;
		ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
		HRESULT InitializeDirectX();
	};
}

#endif