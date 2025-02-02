#pragma once

struct PipeLineInfo
{
	ComPtr<ID3D11Device> m_pDevice = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
};

class PipeLine
{
public:
	PipeLine();
	~PipeLine();

public:
	ComPtr<ID3D11BlendState> GetAlphaBlendState() { return m_pAlphaBlendState; }
	ComPtr<ID3D11BlendState> GetParticleBlendState() { return m_pParticleBlendState; }
	ComPtr<ID3D11DepthStencilState> GetLessEqualDSS() { return m_LessEqualDSS; }
	ComPtr<ID3D11DepthStencilState> GetNoDepthWritesDSS() { return m_pNoDepthWritesDSS; }
	ComPtr<ID3D11DepthStencilState> GetDisableDSS() { return m_pDisableDepthDSS; }
	ComPtr<ID3D11RasterizerState> GetRasterizerStateCCW() { return m_pRasterizerStateCCW; }
	ComPtr<ID3D11RasterizerState> GetRasterizerStateCW() { return m_pRasterizerStateCW; }

public:
	void Initalize(PipeLineInfo info);
	void Render();

private:
	void createSamplerState();
	void createBlendState();
	void createDepthStencilState();
	void createRasterizerState();

private:
	ComPtr<ID3D11Device> m_pDevice = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;

	ComPtr<ID3D11SamplerState> m_pSamplerLinear = nullptr;		// �ؽ�ó ���÷�
	ComPtr<ID3D11SamplerState> m_pSamplerClamp = nullptr;		// �ؽ�ó ���÷�
	ComPtr<ID3D11SamplerState> m_pShadowSampler = nullptr;		// �ؽ�ó ���÷�
	ComPtr<ID3D11BlendState>   m_pAlphaBlendState = nullptr;		// ���� ���� ���� (������ ó���� ���� �������)
	ComPtr<ID3D11BlendState>   m_pParticleBlendState = nullptr;		// ���� ���� ���� (��ƼŬ ó���� ���� �������)

	ComPtr<ID3D11RasterizerState> m_pRasterizerStateCCW; // �� ��� CCW
	ComPtr<ID3D11RasterizerState> m_pRasterizerStateCW; // �� ��� CW
	ComPtr<ID3D11DepthStencilState> m_LessEqualDSS;
	ComPtr<ID3D11DepthStencilState> m_pNoDepthWritesDSS;			// ������ ���� ���ٽ� ������Ʈ ( ���̰� �Ⱦ��� ���̰� ��� )
	ComPtr<ID3D11DepthStencilState> m_pDisableDepthDSS;				// ������ ���� ���ٽ� ������Ʈ ( ���̰� �Ⱦ��� ���̰� �Ȼ�� )
};

