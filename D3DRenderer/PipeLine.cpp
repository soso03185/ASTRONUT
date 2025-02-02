#include "pch.h"
#include "PipeLine.h"

PipeLine::PipeLine()
{
}

PipeLine::~PipeLine()
{
}

void PipeLine::Initalize(PipeLineInfo info)
{
	m_pDevice = info.m_pDevice;
	m_pDeviceContext = info.m_pDeviceContext;

	createSamplerState();
	createBlendState();
	createDepthStencilState();
	createRasterizerState();
}

void PipeLine::Render()
{

}

void PipeLine::createSamplerState()
{
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&sampDesc, m_pSamplerLinear.GetAddressOf()));

	sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.MaxAnisotropy = (sampDesc.Filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&sampDesc, m_pSamplerClamp.GetAddressOf()));

	sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.MaxLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&sampDesc, m_pShadowSampler.GetAddressOf()));

	m_pDeviceContext->VSSetSamplers(0, 1, m_pSamplerLinear.GetAddressOf());
	m_pDeviceContext->GSSetSamplers(0, 1, m_pSamplerLinear.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerLinear.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(1, 1, m_pSamplerClamp.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(2, 1, m_pShadowSampler.GetAddressOf());
}

void PipeLine::createBlendState()
{
	D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = true;						// ���� ��� ����
	// FinalRGB = (SrcRGB * SrcBlend) + (DestRGB * DestBlend)
	rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;	    // SrcBlend�� SrcColor�� ���İ�
	rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;	// DestBlend�� (1-SourceColor.a)
	rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;			// �÷� ���� ������ �����մϴ�.
	// FinalAlpha = (SrcAlpha * SrcBlendAlpha) + (DestAlpha * DestBlendAlpha)
	rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;		// ���� ���� ������ �����մϴ�.
	rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;		// SrcBlendAlpha = 1
	rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;		// DestBlendAlpha = 1	
	rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // ����Ÿ�ٿ� RGBA ��� Write	// ���� ���� Ÿ�ٿ� ���� �÷� ������Ʈ ���� ����ũ�� �����մϴ�.

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;  // ���� ���ø� ��ƿ�����(Anti-aliasing)�� �����ϱ� ���� ���
	blendDesc.IndependentBlendEnable = false; // �� ���� Ÿ�ٿ� ���� �������� ���� ������ ������� ����
	blendDesc.RenderTarget[0] = rtBlendDesc;
	HR_T(m_pDevice->CreateBlendState(&blendDesc, m_pAlphaBlendState.GetAddressOf()));

	//AdditiveBlending
	D3D11_BLEND_DESC additiveBlendingDesc = { 0, };
	additiveBlendingDesc.AlphaToCoverageEnable = false;
	additiveBlendingDesc.IndependentBlendEnable = false;
	additiveBlendingDesc.RenderTarget[0].BlendEnable = true;
	additiveBlendingDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	additiveBlendingDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	additiveBlendingDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	additiveBlendingDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	additiveBlendingDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	additiveBlendingDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	additiveBlendingDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
	HR_T(m_pDevice->CreateBlendState(&additiveBlendingDesc, m_pParticleBlendState.GetAddressOf()));
}

void PipeLine::createDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC lessEqualDesc;
	lessEqualDesc.DepthEnable = true;
	lessEqualDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	lessEqualDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // ���� ������ �� ���� �ѵ� ���
	lessEqualDesc.StencilEnable = false;

	D3D11_DEPTH_STENCIL_DESC noDepthWirtesDesc = {};
	noDepthWirtesDesc.DepthEnable = true;
	noDepthWirtesDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	noDepthWirtesDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	noDepthWirtesDesc.StencilEnable = false;

	D3D11_DEPTH_STENCIL_DESC disableDepthDesc = {};
	disableDepthDesc.DepthEnable = false;
	disableDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	disableDepthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	disableDepthDesc.StencilEnable = false;

	HR_T(m_pDevice->CreateDepthStencilState(&lessEqualDesc, m_LessEqualDSS.GetAddressOf()));
	HR_T(m_pDevice->CreateDepthStencilState(&noDepthWirtesDesc, m_pNoDepthWritesDSS.GetAddressOf()));
	HR_T(m_pDevice->CreateDepthStencilState(&disableDepthDesc, m_pDisableDepthDSS.GetAddressOf()));
}

void PipeLine::createRasterizerState()
{
	HRESULT hr = 0;

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.AntialiasedLineEnable = true;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	HR_T(m_pDevice->CreateRasterizerState(&rasterizerDesc, m_pRasterizerStateCCW.GetAddressOf()));

	rasterizerDesc.FrontCounterClockwise = false;
	HR_T(m_pDevice->CreateRasterizerState(&rasterizerDesc, m_pRasterizerStateCW.GetAddressOf()));
}
