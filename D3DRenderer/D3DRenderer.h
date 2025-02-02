#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <directxtk/BufferHelpers.h>
#include <wrl/client.h>
#include <unordered_map>

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DirectX::SimpleMath;

/// <summary>
/// Renderer�� �̱������� ������ �ʱ����� �̱����� CommonApp(Core)�� Ȱ���Ѵ�.
/// GameApp �ܿ����� �������� ���縦 ����� �ϰ�,
/// Engine������ �������� ���縦 �ȴ�.
/// </summary>

struct TransformCB
{
	Matrix mWorld;	//TODO
	Matrix mView;
	Matrix mProjection;
};

struct LightingCB
{
	Vector4 mLightDirection;
	Vector4 mLightColor;
	Vector3 mCameraTranslation;
	FLOAT mLightIntensity;
	FLOAT mAmbientPower;
	FLOAT mSpecularPower;
	FLOAT garbage1;
	FLOAT garbage2;
};

class D3DRenderer
{
private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGISwapChain> m_pSwapChain;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;

	//TODO : meshInstance�� ���ϴ� vertex, pixel Shader ������ �־ ���̴� ����
	std::unordered_map<const WCHAR*, ComPtr<ID3D11VertexShader>> m_vertexShaderMap;
	std::unordered_map<const WCHAR*, ComPtr<ID3D11PixelShader>> m_pixelShaderMap;

	std::unordered_map<const WCHAR*, ComPtr<ID3D11InputLayout>> m_pInputLayouts;			// �Է� ���̾ƿ�
	ComPtr<ID3D11SamplerState> m_pSamplerLinear;		// ���÷� ����
	ComPtr<ID3D11BlendState> m_pAlphaBlendState;		// ���� ����

	ComPtr<ID3D11Buffer> m_pTransformSubresource;
	ComPtr<ID3D11Buffer> m_pLightingSubresource;
	TransformCB m_transformCB;
	LightingCB m_lightingCB;

	const int m_screenWidth;
	const int m_screenHeight;

	Matrix m_viewMatrix;
	Matrix m_projectionMatrix;

public:
	ComPtr<ID3D11Device> GetDevice() { return m_pDevice; }
	ComPtr<ID3D11DeviceContext> GetDevcon() { return m_pDeviceContext; }
	ComPtr<IDXGISwapChain> GetSwapChain() { return m_pSwapChain; }

	void SetTransformCB(Matrix transform) { m_transformCB.mWorld = transform; }
	void SetViewMatrix(const Matrix& viewMatrix) { m_viewMatrix = viewMatrix; }
	void SetProjectionMatrix(const Matrix& projectionMatrix) { m_projectionMatrix = projectionMatrix; }

public:
	D3DRenderer(int screenWidth, int screenHeight);
	~D3DRenderer();

public:
	bool Initialize(HWND hwnd);
	void Render();
	void Finalize();
};

