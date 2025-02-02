#include "pch.h"
#include "Graphics.h"

Graphics::Graphics()
{
}

Graphics::~Graphics()
{
}

void Graphics::Initalize(GraphicsInfo info)
{
	m_hWnd = info.m_hwnd;
	m_ClientWidth = info.m_ClientWidth;
	m_ClientHeight = info.m_ClientHeight;
	m_ClearColor = info.m_ClearColor;

	setViewPort();
	createDirect2D();
	createSwapChain();
	createRenderTargetView();
	createStencilAndDepth();
}


/// �׸��� ���� Ÿ�� �� �� ���� ���ٽ� �� ���� ///
void Graphics::RenderBeginShadow()
{
	// ���� ���̴� ���ҽ� ������ ����ε�
	ID3D11ShaderResourceView* pSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(11, 1, &pSRV);
	SAFE_RELEASE(pSRV);

	// ���� Ÿ�� �� �� ���� ���ٽ� �� ���� �� Ŭ����
	m_pDeviceContext->RSSetViewports(1, &m_ShadowViewport);
	m_pDeviceContext->OMSetRenderTargets(0, NULL, m_pShadowDepthStencilView.Get());
	m_pDeviceContext->ClearDepthStencilView(m_pShadowDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0);
	m_pDeviceContext->PSSetShader(NULL, NULL, 0);
}


/// �ܰ��� �β� ���� Ÿ�� �� �� ���� ���ٽ� �� ���� ///
void Graphics::RenderBeginOutLineOver()
{
	// ���� ���̴� ���ҽ� ������ ����ε�
	ComPtr<ID3D11ShaderResourceView> pSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(15, 1, pSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(16, 1, pSRV.GetAddressOf());

	// ���� Ÿ�� �� �� ���� ���ٽ� �� ���� �� Ŭ����
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetRenderTargets(0, NULL, m_pOutLineOverDepthStencilView.Get());
	m_pDeviceContext->ClearDepthStencilView(m_pOutLineOverDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0);
	m_pDeviceContext->PSSetShader(NULL, NULL, 0);
}


/// �ܰ��� ���� Ÿ�� �� �� ���� ���ٽ� �� ���� ///
void Graphics::RenderBeginOutLine()
{
	// ���� ���̴� ���ҽ� ������ ����ε�
	ComPtr<ID3D11ShaderResourceView> pSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(15, 1, pSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(16, 1, pSRV.GetAddressOf());

	// ���� Ÿ�� �� �� ���� ���ٽ� �� ���� �� Ŭ����
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetRenderTargets(0, NULL, m_pOutLineDepthStencilView.Get());
	m_pDeviceContext->ClearDepthStencilView(m_pOutLineDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0);
	m_pDeviceContext->PSSetShader(NULL, NULL, 0);
}


/// 3D UI Mesh ���� Ÿ�� �� �� ���� ���ٽ� �� ���� ///
void Graphics::RenderBeginUIMesh_1()
{
	// ���� ���̴� ���ҽ� ������ ����ε�
	ComPtr<ID3D11ShaderResourceView> pSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(18, 1, pSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(19, 1, pSRV.GetAddressOf());

	// ���� Ÿ�� �� �� ���� ���ٽ� �� ���� �� Ŭ����
	float color[4] = { 0.f, 0.f, 0.f, 0.f };
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetRenderTargets(1, m_pUIMesh_1_RenderTargetView.GetAddressOf(), m_pUIMeshDepthStencilView.Get());
	m_pDeviceContext->ClearRenderTargetView(m_pUIMesh_1_RenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pUIMeshDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pDeviceContext->PSSetShader(NULL, NULL, 0);
}


/// 3D UI Mesh ���� Ÿ�� �� �� ���� ���ٽ� �� ���� ///
void Graphics::RenderBeginUIMesh_2()
{
	// ���� ���̴� ���ҽ� ������ ����ε�
	ComPtr<ID3D11ShaderResourceView> pSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(18, 1, pSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(19, 1, pSRV.GetAddressOf());

	// ���� Ÿ�� �� �� ���� ���ٽ� �� ���� �� Ŭ����
	float color[4] = { 0.f, 0.f, 0.f, 0.f };
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetRenderTargets(1, m_pUIMesh_2_RenderTargetView.GetAddressOf(), m_pUIMeshDepthStencilView.Get());
	m_pDeviceContext->ClearRenderTargetView(m_pUIMesh_2_RenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pUIMeshDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pDeviceContext->PSSetShader(NULL, NULL, 0);
}


/// 3D ���� Ÿ�� �� �� ���� ���ٽ� �� ���� ///
void Graphics::RenderBegin()
{
	// ���� ���̴� ���ҽ� ������ ����ε�
	ID3D11ShaderResourceView* pSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(17, 1, &pSRV);
	SAFE_RELEASE(pSRV);

	// ���� Ÿ�� �� �� ���� ���ٽ� �� ���� �� Ŭ����
	float color[4] = { m_ClearColor.x, m_ClearColor.y, m_ClearColor.z, m_ClearColor.w };
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}


/// ���� ���� Ÿ�� �� �� ���� ���ٽ� �� ���� ///
void Graphics::RenderBeginPostProcessing()
{
	// ���� Ÿ�� �� �� ���� ���ٽ� �� ���� �� Ŭ����
	float color[4] = { m_ClearColor.x, m_ClearColor.y, m_ClearColor.z, m_ClearColor.w };
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetRenderTargets(1, m_pFinalRenderTargetView.GetAddressOf(), m_pFinalDepthStencilView.Get());
	m_pDeviceContext->ClearRenderTargetView(m_pFinalRenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pFinalDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// �� �ؽ��� ����
	m_pDeviceContext->PSSetShaderResources(15, 1, m_pOutLineResourceView.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(16, 1, m_pOutLineOverResourceView.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(17, 1, m_pRenderResourceView.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(18, 1, m_pUIMesh_1_RenderResourceView.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(19, 1, m_pUIMesh_2_RenderResourceView.GetAddressOf());

	//d2d
	GetD2DRenderTarget()->BeginDraw();
}

void Graphics::RenderEnd()
{
	//d2d
	GetD2DRenderTarget()->EndDraw();

	HRESULT hr = m_pSwapChain->Present(0, 0);
	assert(SUCCEEDED(hr));
}

void Graphics::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC swapDesc = {};

	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// ����ü�� ����� â �ڵ� ��.
	swapDesc.Windowed = true;		// â ��� ���� ����.
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// �����(�ؽ�ó)�� ����/���� ũ�� ����.
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	// ȭ�� �ֻ��� ����.

	swapDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapDesc.BufferDesc.RefreshRate.Denominator = 0;
	// ���ø� ���� ����.
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	swapDesc.BufferCount = 2;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// Present()ȣ�� �� ���÷��� ȭ�鿡�� �ȼ��� ó���ϴ� �ɼ�
	swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT creationFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT; //D2D ������ BGRA ����Ʈ �������
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 1. ��ġ ����.   2.����ü�� ����. 3.��ġ ���ؽ�Ʈ ����.
	HR_T(D3D11CreateDeviceAndSwapChain
	(
		  NULL
		, D3D_DRIVER_TYPE_HARDWARE
		, NULL
		, creationFlags
		, NULL
		, NULL
		, D3D11_SDK_VERSION
		, &swapDesc
		, &m_pSwapChain
		, &m_pDevice
		, NULL
		, &m_pDeviceContext)
	);
}

void Graphics::createDirect2D()
{
	HR_T(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(m_pdWriteFactory.GetAddressOf())
	));

	HR_T(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_pd2dFactory.GetAddressOf()));
}

void Graphics::createRenderTargetView()
{
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = m_ClientWidth;
	descDepth.Height = m_ClientHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// D3D Render Target
	{
		// ���� �������� 3D Model���� ��� ���� Ÿ�� ��
		ComPtr<ID3D11Texture2D> pBackBufferTexture = nullptr;
		HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &pBackBufferTexture));
		HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), NULL, m_pRenderTargetView.GetAddressOf()));  // �ؽ�ó�� ���� ���� ����
		HR_T(m_pDevice->CreateShaderResourceView(pBackBufferTexture.Get(), &srvDesc, m_pRenderResourceView.GetAddressOf()));

		// 3D Mesh�� ���� �������� ���� �ؽ���ó�� ����� ���� Ÿ�� ��
		ComPtr<ID3D11Texture2D> pUIMesh_1_BufferTexture = nullptr;
		HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &pUIMesh_1_BufferTexture));
		HR_T(m_pDevice->CreateRenderTargetView(pUIMesh_1_BufferTexture.Get(), NULL, m_pUIMesh_1_RenderTargetView.GetAddressOf()));  // �ؽ�ó�� ���� ���� ����
		HR_T(m_pDevice->CreateShaderResourceView(pUIMesh_1_BufferTexture.Get(), &srvDesc, m_pUIMesh_1_RenderResourceView.GetAddressOf()));

		// 3D Mesh�� ���� �������� ���� �ؽ���ó�� ����� ���� Ÿ�� ��
		ComPtr<ID3D11Texture2D> pUIMesh_2_BufferTexture = nullptr;
		HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &pUIMesh_2_BufferTexture));
		HR_T(m_pDevice->CreateRenderTargetView(pUIMesh_2_BufferTexture.Get(), NULL, m_pUIMesh_2_RenderTargetView.GetAddressOf()));  // �ؽ�ó�� ���� ���� ����
		HR_T(m_pDevice->CreateShaderResourceView(pUIMesh_2_BufferTexture.Get(), &srvDesc, m_pUIMesh_2_RenderResourceView.GetAddressOf()));

		// ���� ���� Ÿ�� ��
		ComPtr<ID3D11Texture2D> pfinalBufferTexture = nullptr;
		HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pfinalBufferTexture));
		HR_T(m_pDevice->CreateRenderTargetView(pfinalBufferTexture.Get(), NULL, m_pFinalRenderTargetView.GetAddressOf()));  // �ؽ�ó�� ���� ���� ����
		HR_T(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(m_pdxgiBackbuffer.GetAddressOf())));
	}

	// D2D Render Target
	{
		D2D1_RENDER_TARGET_PROPERTIES props =
			D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
				0,
				0);

		HR_T(m_pd2dFactory->CreateDxgiSurfaceRenderTarget(
			m_pdxgiBackbuffer.Get(),
			props,
			m_pd2dRenderTargetView.GetAddressOf()));
	}
}

void Graphics::createStencilAndDepth()
{
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = m_ClientWidth;
	descDepth.Height = m_ClientHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	ID3D11Texture2D* textureDepthStencil = nullptr;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &textureDepthStencil));

	HR_T(m_pDevice->CreateDepthStencilView(textureDepthStencil, &descDSV, m_pDepthStencilView.GetAddressOf()));
	SAFE_RELEASE(textureDepthStencil);

	ID3D11Texture2D* uiMeshDepthStencil = nullptr;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &uiMeshDepthStencil));

	HR_T(m_pDevice->CreateDepthStencilView(uiMeshDepthStencil, &descDSV, m_pUIMeshDepthStencilView.GetAddressOf()));
	SAFE_RELEASE(uiMeshDepthStencil);

	ID3D11Texture2D* finalTextureDepthStencil = nullptr;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &finalTextureDepthStencil));

	HR_T(m_pDevice->CreateDepthStencilView(finalTextureDepthStencil, &descDSV, m_pFinalDepthStencilView.GetAddressOf()));
	SAFE_RELEASE(finalTextureDepthStencil);

	// �׸��ڿ� ShadowMap Texture�� SRV, DSV ����
	{
		// Create Shadow map texture desc
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = static_cast<float>(SHADOWMAP_SIZE);
		texDesc.Height = static_cast<float>(SHADOWMAP_SIZE);
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;

		ID3D11Texture2D* pShadowMap = nullptr;
		HR_T(m_pDevice->CreateTexture2D(&texDesc, NULL, &pShadowMap));

		// Create the depth stencil view desc
		D3D11_DEPTH_STENCIL_VIEW_DESC ShadowdescDSV = {};
		ShadowdescDSV.Format = DXGI_FORMAT_D32_FLOAT;
		ShadowdescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		HR_T(m_pDevice->CreateDepthStencilView(pShadowMap, &ShadowdescDSV, m_pShadowDepthStencilView.GetAddressOf()));

		// Create the depth stencil view desc
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		HR_T(m_pDevice->CreateShaderResourceView(pShadowMap, &srvDesc, m_pShadowResourceView.GetAddressOf()));
		SAFE_RELEASE(pShadowMap);
	}
	// �ܰ����� OverLine Texture�� SRV, DSV ����
	{
		// Create Shadow map texture desc
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = m_ClientWidth;
		texDesc.Height = m_ClientHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;

		ID3D11Texture2D* pOutLineOverMap = nullptr;
		ID3D11Texture2D* pOutLineMap = nullptr;
		HR_T(m_pDevice->CreateTexture2D(&texDesc, NULL, &pOutLineOverMap));
		HR_T(m_pDevice->CreateTexture2D(&texDesc, NULL, &pOutLineMap));

		// Create the depth stencil view desc
		D3D11_DEPTH_STENCIL_VIEW_DESC OutLineDSV = {};
		OutLineDSV.Format = DXGI_FORMAT_D32_FLOAT;
		OutLineDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		

		// Create the Shader Resource View Desc
		D3D11_SHADER_RESOURCE_VIEW_DESC OutLineSRV = {};
		OutLineSRV.Format = DXGI_FORMAT_R32_FLOAT;
		OutLineSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		OutLineSRV.Texture2D.MipLevels = 1;

		HR_T(m_pDevice->CreateDepthStencilView(pOutLineOverMap, &OutLineDSV, m_pOutLineOverDepthStencilView.GetAddressOf()));
		HR_T(m_pDevice->CreateShaderResourceView(pOutLineOverMap, &OutLineSRV, m_pOutLineOverResourceView.GetAddressOf()));
		HR_T(m_pDevice->CreateDepthStencilView(pOutLineMap, &OutLineDSV, m_pOutLineDepthStencilView.GetAddressOf()));
		HR_T(m_pDevice->CreateShaderResourceView(pOutLineMap, &OutLineSRV, m_pOutLineResourceView.GetAddressOf()));

		SAFE_RELEASE(pOutLineOverMap);
		SAFE_RELEASE(pOutLineMap);
	}
}

void Graphics::setViewPort()
{
	m_Viewport.TopLeftX = 0.f;
	m_Viewport.TopLeftY = 0.f;
	m_Viewport.Width = static_cast<float>(m_ClientWidth);
	m_Viewport.Height = static_cast<float>(m_ClientHeight);
	m_Viewport.MinDepth = 0.f;
	m_Viewport.MaxDepth = 1.f;

	m_ShadowViewport.TopLeftX = 0.f;
	m_ShadowViewport.TopLeftY = 0.f;
	m_ShadowViewport.Width = static_cast<float>(SHADOWMAP_SIZE);
	m_ShadowViewport.Height = static_cast<float>(SHADOWMAP_SIZE);
	m_ShadowViewport.MinDepth = 0.f;
	m_ShadowViewport.MaxDepth = 1.f;
}
