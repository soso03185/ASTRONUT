#pragma once

#include "ParticleShader.h"

struct CB_PerFrame
{
	Matrix ViewProjection;				// ī�޶� ��������
	Vector3 EyePosWorld;				// ī�޶� ���� ������
	float GameTime;						// ���� �ð�
	Vector3 EmitPosWorld;				// �̹̼� ���� ������
	float TimeStep;						// ��Ÿ Ÿ��
	Vector3 EmitGenerationRange;		// �̹̼� ���� ���� ũ��
	float Pad;
};

struct CB_ParticleData
{
	Vector2 Size;						// ũ��
	float   CreatePerSecond;			// 1�ʴ� ���� ����
	float	LifeTime;					// ��ƼŬ ���� �ð�
	Vector3 InitialSpeed;				// �ʱ� �ӵ�
	int		PlayParticle;				// ��ƼŬ ���
	Vector3 Acceleration;				// ���ӵ�
	int		ParticleDirection;			// ��ƼŬ ����
};

struct ParticleVertex
{
	Vector3 InitialPosition;
	Vector3 InitialVelocity;
	Vector2 Size;
	float Age;
	unsigned int Type;
};

enum class eParticleDirection
{
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
};

class PipeLine;

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

public:
	const std::wstring& GetTextureName() { return m_TexturePath; }
	float GetAge() const { return m_Age; }
	void SetEyePos(const Vector3& eyePosWorld) { m_CBPerFrame.EyePosWorld = eyePosWorld; }
	void SetEmitPos(const Vector3& emitPosWorld) { m_CBPerFrame.EmitPosWorld = emitPosWorld; }
	void SetEmitGenerationRange(const Vector3& emitGenerationRange) { m_CBPerFrame.EmitGenerationRange = emitGenerationRange; }
	void SetParticleSize(const Vector2& particleSize) { m_CBParticleData.Size = particleSize; }
	void SetParticleInitialSpeed(const Vector3& initialSpeed) { m_CBParticleData.InitialSpeed = initialSpeed; }
	void SetParticleAcceleration(const Vector3& acceleration) { m_CBParticleData.Acceleration = acceleration; }
	void SetParticleCreatePerSecond(const float& createPerSecond) { m_CBParticleData.CreatePerSecond = createPerSecond; }
	void SetParticleLifeTime(const float& lifeTime) { m_CBParticleData.LifeTime = lifeTime; }
	void SetPlayParticle(const bool& isPlay) { m_CBParticleData.PlayParticle = isPlay; }
	void SetParticleDirection(const int& ParticleDirection) { m_CBParticleData.ParticleDirection = ParticleDirection; }

public:
	void Init(std::wstring TexturePath, UINT maxParticles, eParticleType particleType, std::wstring shaderPath);
	void Reset();
	void Update(float delayTime, float gameTime);
	void Render(ComPtr<ID3D11DeviceContext> deviceContext, std::weak_ptr<PipeLine> pipeLine);

private:
	void textureSetting(ComPtr<ID3D11DeviceContext> deviceContext);
	void streamOutPipeLineSetting(ComPtr<ID3D11DeviceContext> deviceContext, std::weak_ptr<PipeLine> pipeLine);
	void drawPipeLineSetting(ComPtr<ID3D11DeviceContext> deviceContext, std::weak_ptr<PipeLine> pipeLine);
	void endPipeLineSetting(ComPtr<ID3D11DeviceContext> deviceContext, std::weak_ptr<PipeLine> pipeLine);

public:
	void UpdatePerFrame(CB_PerFrame& perFrame);
	void UpdatePerticleData(CB_ParticleData& particleData);

private:
	eParticleType m_ParticleType;
	CB_PerFrame m_CBPerFrame;
	CB_ParticleData m_CBParticleData;

	UINT m_MaxParticles;
	bool m_FirstRun;

	float m_Age;

private:
	UINT m_Stride;
	UINT m_Offset;

	// �ؽ��� �̸�
	std::wstring m_ShaderPath;
	std::wstring m_TexturePath;
	ComPtr<ID3D11ShaderResourceView> m_RandomTexture;

	// ����
	ComPtr<ID3D11Buffer> m_InitVertexBuffer;
	ComPtr<ID3D11Buffer> m_DrawVertexBuffer;
	ComPtr<ID3D11Buffer> m_StreamOutVertexBuffer;
};