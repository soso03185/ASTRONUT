#include "pch.h"
#include "ParticleSystem.h"

#include "Helper.h"
#include "D3DRenderManager.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "PipeLine.h"

ParticleSystem::ParticleSystem()
	: m_FirstRun(true)
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Init(wstring TexturePath, UINT maxParticles, eParticleType particleType, wstring shaderPath)
{
	m_TexturePath = TexturePath;
	m_ParticleType = particleType;
	m_MaxParticles = maxParticles;
	m_ShaderPath = shaderPath;
	m_FirstRun = true;
	m_RandomTexture = CreateRandomTexture1DSRV(D3DRenderManager::m_pDevice.Get());

	// ��� ���� ����
	ParticleVertex p;
	ZeroMemory(&p, sizeof(ParticleVertex));
	p.Age = 0.0f;
	p.Type = 0;

	// ��Ʈ�� ��¿� ���� ���� �����
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(ParticleVertex) * 1;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;
	HR_T(D3DRenderManager::m_pDevice->CreateBuffer(&vbd, &vinitData, &m_InitVertexBuffer)); // �ʱ�ȭ�� ����� ����

	vbd.ByteWidth = sizeof(ParticleVertex) * m_MaxParticles; // �ִ�� �ٷ� ���� ũ��, �Ѿ�� �ȴ�
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT; // �÷��� �߿�!
	HR_T(D3DRenderManager::m_pDevice->CreateBuffer(&vbd, 0, &m_DrawVertexBuffer));
	HR_T(D3DRenderManager::m_pDevice->CreateBuffer(&vbd, 0, &m_StreamOutVertexBuffer));
}

void ParticleSystem::Reset()
{
	m_FirstRun = true;
	m_Age = 0.0f;
}

void ParticleSystem::Update(float delayTime, float gameTime)
{
	m_CBPerFrame.GameTime = gameTime;
	m_CBPerFrame.TimeStep = delayTime;
	m_Age += delayTime;
}

void ParticleSystem::Render(ComPtr<ID3D11DeviceContext> deviceContext, weak_ptr<PipeLine> pipeLine)
{
	m_Stride = sizeof(ParticleVertex);
	m_Offset = 0;

	textureSetting(deviceContext);
	streamOutPipeLineSetting(deviceContext, pipeLine);

	// ����
	// ó�� �׸� ���� ���� �ϳ��� �׸���. �ƴϸ� ���� ������ŭ �˾Ƽ� draw ���ش�.
	if (m_FirstRun) { deviceContext->Draw(1, 0); m_FirstRun = false; }
	else			{ deviceContext->DrawAuto(); }

	drawPipeLineSetting(deviceContext, pipeLine);
	deviceContext->DrawAuto();
	endPipeLineSetting(deviceContext, pipeLine);
}

void ParticleSystem::textureSetting(ComPtr<ID3D11DeviceContext> deviceContext)
{
	// ��ƼŬ ���̴� �� �ؽ��� �ҷ�����
	auto texture = ResourceManager::GetInstance()->CreateTextureResource(m_TexturePath.c_str());

	// ���ҽ� �� ���������� ����
	deviceContext->VSSetShaderResources(20, 1, texture->GetTexture().GetAddressOf());
	deviceContext->VSSetShaderResources(21, 1, m_RandomTexture.GetAddressOf());
	deviceContext->GSSetShaderResources(20, 1, texture->GetTexture().GetAddressOf());
	deviceContext->GSSetShaderResources(21, 1, m_RandomTexture.GetAddressOf());
	deviceContext->PSSetShaderResources(20, 1, texture->GetTexture().GetAddressOf());
	deviceContext->PSSetShaderResources(21, 1, m_RandomTexture.GetAddressOf());
}

void ParticleSystem::streamOutPipeLineSetting(ComPtr<ID3D11DeviceContext> deviceContext, weak_ptr<PipeLine> pipeLine)
{
	auto shader = ResourceManager::GetInstance()->CreateParticleShader(m_ShaderPath, m_ParticleType);

	// �ε��� ���� ���� ����
	// �������� ���� ( ����Ʈ )
	// ���� ���ٽ� ������Ʈ ���� ( ���� ���� ����, ���� ���� ��� ���� )
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->OMSetDepthStencilState(pipeLine.lock()->GetDisableDSS().Get(), 0);	// ���� ���� ����

	// ��Ʈ�� �ƿ����� ���� ����/����
	shader->StreamOutShaderSetting(deviceContext.Get());
	deviceContext->SOSetTargets(1					// ���� ���� ����, �ִ� 4��
		, m_StreamOutVertexBuffer.GetAddressOf()	// ��¿� ����
		, &m_Offset);									// ���� ���۸��� ����ϱ� ������ ��ġ�� ��Ÿ��, ���� 0

	// ���ε�
	// ó�� �׸� �� �ʱ�ȭ ���۷� �׸���. �ƴϸ� �׳� �׸���� ���� ���.
	if (m_FirstRun)	
	{
		deviceContext->IASetVertexBuffers(0, 1, m_InitVertexBuffer.GetAddressOf(), &m_Stride, &m_Offset);
	}
	else			
	{
		deviceContext->IASetVertexBuffers(0, 1, m_DrawVertexBuffer.GetAddressOf(), &m_Stride, &m_Offset);
	}
}

void ParticleSystem::drawPipeLineSetting(ComPtr<ID3D11DeviceContext> deviceContext, weak_ptr<PipeLine> pipeLine)
{
	auto shader = ResourceManager::GetInstance()->CreateParticleShader(m_ShaderPath, m_ParticleType);
	ID3D11Buffer* bufferArray[1] = { NULL };
	float factor[] = { 0.f, 0.f, 0.f, 0.f };

	// ��ο� ���������� ����
	// ���� ������Ʈ ( ���� ȥ�� )
	// ���� ���ٽ� ������Ʈ ( ���� ���� ���, ���� ���� )
	// ���ؽ� ���� ���� ( ��ο� ���ؽ� ���� )
	shader->DrawShaderSetting(deviceContext.Get());
	deviceContext->OMSetBlendState(pipeLine.lock()->GetParticleBlendState().Get(), factor, 0xffffffff);
	deviceContext->OMSetDepthStencilState(pipeLine.lock()->GetNoDepthWritesDSS().Get(), 0);
	deviceContext->IASetVertexBuffers(0, 1, m_DrawVertexBuffer.GetAddressOf(), &m_Stride, &m_Offset);

	// 
	deviceContext->SOSetTargets(1, bufferArray, &m_Offset);	// ��� �ܰ踦 ����ش�.
	swap(m_DrawVertexBuffer, m_StreamOutVertexBuffer);		// ������ ���� swap �Ѵ�.
}

void ParticleSystem::endPipeLineSetting(ComPtr<ID3D11DeviceContext> deviceContext, weak_ptr<PipeLine> pipeLine)
{
	auto shader = ResourceManager::GetInstance()->CreateParticleShader(m_ShaderPath, m_ParticleType);
	float factor[] = { 0.f, 0.f, 0.f, 0.f };

	// �ʱ�ȭ ���� �ϱ�
	// ���̴� �ʱ�ȭ
	// ���� ������Ʈ �ʱ�ȭ
	// ���� ���ٽ� ������Ʈ ��� ����, ������� ����
	shader->NullShaderSetting(deviceContext.Get());
	deviceContext->OMSetBlendState(0, factor, 0xffffffff);
	deviceContext->OMSetDepthStencilState(pipeLine.lock()->GetLessEqualDSS().Get(), 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleSystem::UpdatePerFrame(CB_PerFrame& perFrame)
{
	perFrame.GameTime = m_CBPerFrame.GameTime;
	perFrame.TimeStep = m_CBPerFrame.TimeStep;
	perFrame.EmitGenerationRange = m_CBPerFrame.EmitGenerationRange;
	perFrame.EmitPosWorld = m_CBPerFrame.EmitPosWorld;
	perFrame.EyePosWorld = m_CBPerFrame.EyePosWorld;
}

void ParticleSystem::UpdatePerticleData(CB_ParticleData& particleData)
{
	particleData.Acceleration = m_CBParticleData.Acceleration;
	particleData.CreatePerSecond = m_CBParticleData.CreatePerSecond;
	particleData.InitialSpeed = m_CBParticleData.InitialSpeed;
	particleData.LifeTime = m_CBParticleData.LifeTime;
	particleData.ParticleDirection = m_CBParticleData.ParticleDirection;
	particleData.PlayParticle = m_CBParticleData.PlayParticle;
	particleData.Size = m_CBParticleData.Size;
}
