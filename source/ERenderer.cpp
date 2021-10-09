#include "pch.h"

//Project includes
#include "ERenderer.h"
#include "SceneGraph.h"
#include "Triangle.h"

Elite::Renderer::Renderer(SDL_Window * pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
{
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);

	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBuffer = new float[size_t(m_Width) * size_t(m_Height)];

	std::cout << "Initializing DirectX" << '\n';
	HRESULT result = InitializeDirectX();
	if (FAILED(result))
	{
		std::cout << "Initializing DirectX FAILED" << '\n';
	}
	else
	{
		m_IsInitialized = true;
		std::cout << "DirectX is ready\n";
	}

}

Elite::Renderer::~Renderer()
{
	m_pRenderTargetView->Release();
	m_pRenderTargetBuffer->Release();
	m_pDepthStencilView->Release();
	m_pDepthStencilBuffer->Release();
	m_pSwapChain->Release();
	m_pDXGIFactory->Release();
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}
	m_pDevice->Release();
	delete m_pDepthBuffer;
}

void Elite::Renderer::Render(Camera* pCamera)
{
	if (!m_IsInitialized) 
		return;

	pCamera->UpdateFOV();

	RGBColor clearColor = RGBColor{ 0.f, 0.f, 0.3f };
	
	if (m_useDirectX)
	{
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}
	else
	{
		SDL_LockSurface(m_pBackBuffer);
		auto clearColorARGB = Elite::GetSDL_ARGBColor(clearColor);
		size_t amount = size_t(m_Width) * size_t(m_Height);
		std::fill_n(m_pBackBufferPixels, amount, clearColorARGB);
		std::fill_n(m_pDepthBuffer, amount, FLT_MAX);
	}

	//Render Meshes
	std::vector<Mesh*>& meshes = SceneGraph::GetInstance()->GetMeshes();
	for (Mesh* pMesh : meshes) RenderMesh(pMesh, pCamera);


	if (m_useDirectX)
	{
		m_pSwapChain->Present(0, 0);
	}
	else 
	{
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}
	
}

bool Elite::Renderer::ToggleDirectXRasterizer()
{
	m_useDirectX = !m_useDirectX;
	return m_useDirectX;
}

ID3D11Device* Elite::Renderer::GetDevice()
{
	return m_pDevice;
}

void Elite::Renderer::RenderMesh(Mesh* pMesh, const Camera* pCamera)
{
	pMesh->Update(pCamera);

	if (m_useDirectX)
	{
		auto vertexLayout = pMesh->GetInputLayout();
		auto vertexBuffer = pMesh->GetVertexBufferGPU();
		auto indexBuffer = pMesh->GetIndexBufferGPU();
		auto effect = pMesh->GetEffect();
		int amountIndices = pMesh->GetAmountOfIndices();

		UINT stride = sizeof(Mesh::Vertex_Input);
		UINT offset = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

		//Set index buffer
		m_pDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//Set the input layout
		m_pDeviceContext->IASetInputLayout(vertexLayout);

		//Set primitive topology
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//Render the mesh
		D3DX11_TECHNIQUE_DESC techDesc;
		effect->GetTechnique()->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			effect->GetTechnique()->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
			m_pDeviceContext->DrawIndexed(amountIndices, 0, 0);
		}
	}
	else
	{
		auto& vertexBuffer = pMesh->GetVertexBuffer();
		auto& indexBuffer = pMesh->GetIndexBuffer();
		Elite::FMatrix4 meshWorldMatrix{ pMesh->GetWorldMatrix() };
		meshWorldMatrix[3][2] *= -1; //Invert Z component because it's defined in LH space
		Elite::FMatrix4 worldViewProj{ pCamera->GetProjection() * pCamera->GetView() * meshWorldMatrix };
		for (size_t i = 0; i < indexBuffer.size(); i+=3)
		{
			//Setup triangle
			int i0 = indexBuffer[i];
			int i1 = indexBuffer[i + 1];
			int i2 = indexBuffer[i + 2];

			Triangle triangle{ vertexBuffer[i0],vertexBuffer[i1],vertexBuffer[i2] };
			triangle.UpdateProjectionSpace(worldViewProj);

			//Culling
			if (triangle.IsFrustumCulled(pCamera)) continue;

			const BaseEffect::EffectCullMode& cullMode{ pMesh->GetCullMode() };
			if (cullMode != BaseEffect::EffectCullMode::None)
			{
				const Elite::FPoint3 triangleMiddle{ triangle.GetTriangleMiddle(meshWorldMatrix) };
				const Elite::FVector3 viewDirection{ triangleMiddle - pCamera->GetPosition() };
				float dotViewDirectionVertexNormal{ Elite::Dot(viewDirection, triangle.GetTriangleNormal(meshWorldMatrix)) };

				if (cullMode == BaseEffect::EffectCullMode::Back && dotViewDirectionVertexNormal > 0) continue;
				if (cullMode == BaseEffect::EffectCullMode::Front && dotViewDirectionVertexNormal < 0) continue;
			}

			//Bounding box
			Elite::FPoint2 topLeft{};
			Elite::FPoint2 bottomRight{};
			triangle.GetBoundingBox(topLeft, bottomRight, float(m_Width), float(m_Height));

			//Loop over all the pixels in the bounding box
			for (uint32_t r = uint32_t(topLeft.y); r <= bottomRight.y; ++r)
			{
				for (uint32_t c = uint32_t(topLeft.x); c <= bottomRight.x; ++c)
				{
					uint32_t pixelIndex{ c + (r * m_Width) };
					Elite::FPoint2 screenSpace{ float(c), float(r) };
					Triangle::VertexOut vertexColor{};
					float weight0{}, weight1{}, weight2{};

					if (triangle.Hit(screenSpace, pCamera->GetScreenWidth(), pCamera->GetScreenHeight(), cullMode == BaseEffect::EffectCullMode::Front, vertexColor, weight0, weight1, weight2))
					{
						//Depth test
						if (abs(vertexColor.position.z) >= abs(m_pDepthBuffer[pixelIndex])) continue;

						m_pDepthBuffer[pixelIndex] = vertexColor.position.z;
						triangle.Interpolate(meshWorldMatrix, vertexColor, weight0, weight1, weight2);

						Elite::FVector3 viewDirection{ vertexColor.worldPosition - pCamera->GetPosition() };
						Elite::Normalize(viewDirection);
						Elite::RGBColor shadedColor = PixelShade(pMesh, vertexColor, viewDirection);
						shadedColor.MaxToOne();
						m_pBackBufferPixels[pixelIndex] = Elite::GetSDL_ARGBColor(shadedColor);
					}
				}
			}
		}
	}
}

Elite::RGBColor Elite::Renderer::PixelShade(const Mesh* pMesh, const Triangle::VertexOut& vertex, const Elite::FVector3& viewDirection) const
{
	Elite::FVector3 vertexNormal{ vertex.normal };
	const Texture* pNormal = pMesh->GetNormal();
	//If normal map, update normal to sampled normal
	if (pNormal != nullptr)
	{
		//Calculate Tangent space
		Elite::FVector3 interpolatedBinormal{ Elite::Cross(vertex.tangent, vertex.normal) };
		Elite::FMatrix3 tangentSpaceAxis{ vertex.tangent, interpolatedBinormal, vertex.normal };

		//Sample from normal map
		Elite::RGBColor sampleColor{ pNormal->Sample(vertex.uv) };
		Elite::FVector3 sampleNormal{ sampleColor.r, sampleColor.g, sampleColor.b };
		sampleNormal = 2.f * sampleNormal - Elite::FVector3{ 1.f, 1.f, 1.f };
		vertexNormal = tangentSpaceAxis * sampleNormal;
	}

	Elite::FVector3 lightDirection{ .577f,-.577f,-.577f };
	Elite::RGBColor lightColor{ 1.f,1.f,1.f };
	float lightIntensity{ 2.f };

	float dotLightNormal{ Elite::Dot(-vertexNormal, lightDirection) };
	if (dotLightNormal <= 0) return Elite::RGBColor{};

	const Texture* pDiffuse = pMesh->GetDiffuse();
	const Texture* pSpecular = pMesh->GetSpecular();
	const Texture* pGloss = pMesh->GetGlossiness();

	Elite::RGBColor diffuseSample = vertex.color;
	if (pDiffuse != nullptr) diffuseSample = pDiffuse->Sample(vertex.uv);

	Elite::RGBColor diffuse{ lightColor * lightIntensity * dotLightNormal * diffuseSample };

	//No Phong
	if (pSpecular == nullptr || pGloss == nullptr)
		return diffuse;

	Elite::RGBColor specularColor = pSpecular->Sample(vertex.uv);
	float gloss = pGloss->Sample(vertex.uv).r;
	float shininess{ 25.f };

	Elite::FVector3 reflect{ lightDirection - 2 * (Elite::Dot(vertexNormal, lightDirection) * vertexNormal) };
	Elite::Normalize(reflect);
	float cosAlpha{ Elite::Dot(reflect, -viewDirection) };
	cosAlpha = Elite::Clamp(cosAlpha, 0.f, 1.f);
	float specularReflection{ pow(cosAlpha, gloss * shininess) };

	return diffuse + specularColor * specularReflection;
}

HRESULT Elite::Renderer::InitializeDirectX()
{
	//Create Device and Device context, using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif 
	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);
	if (FAILED(result))
		return result;

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));
	if (FAILED(result))
		return result;

	//Create SwapChain Descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	//Get the handle HWND from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;
	//Create SwapChain and hook it into the handle of the SDL window
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
		return result;

	//Create the Depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	//Create buffer (actual data on GPU)
	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	//Create Render Target View (corresponds to how data is used)
	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, NULL, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//Bind the Views to the Output Merget Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return result;
}


