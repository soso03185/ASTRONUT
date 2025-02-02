#pragma once
#include "RenderComponent.h"

#include "../D3DRenderer/ParticleSystem.h"

/// Particle Component�� �ʿ��� ������ ����ü ///
struct ParticleComponentInfo
{
	Vector3 InitialSpeed;			// ��ƼŬ �ʱ� �ӵ�
	Vector3 Acceleration;			// ��ƼŬ ���ӵ�
	Vector2 Size;					// ��ƼŬ ũ��
	Vector3 GenerationRange;		// ��ƼŬ ���� ����
	float CreatePerSecond;			// ��ƼŬ 1�ʴ� ���� ����
	float LifeTime;					// ��ƼŬ ���� �ð�
	float CreateTime;				// ��ƼŬ ���� �ð�
	std::wstring m_ShaderPath;		// ��ƼŬ ���̴� ���
	std::wstring m_TexturePath;		// ��ƼŬ �ؽ��� ���
	eParticleType m_ParticleType;	// ��ƼŬ Ÿ��
	bool bIsLoop = true;			// ��ƼŬ ����
	int ParticleDirection = 0;		// ��ƼŬ ����
};


/// Particle Component ///
class ParticleComponent :
    public RenderComponent
{
public:
	ParticleComponent() = default;
	~ParticleComponent() override;

public:
	std::shared_ptr<ParticleSystem> GetParticle() const { return m_pParticleSystem; }

	// CreateComponent �� �ʼ������� filePath ����
	void Setting(ParticleComponentInfo particleInfo) { m_ParticleInfo = particleInfo; }


public:
	void Initialize() override;	// staticMeshModel�� �������� ���� ����, �޾ƿ´�.
	void Update(float deltaTime) override;

public:
	void PlayParticle(float particlePlayTime);
	void PlayParticle(bool isPlaying);

private:
	ParticleComponentInfo m_ParticleInfo;				// ��ƼŬ ����
	std::shared_ptr<ParticleSystem> m_pParticleSystem;

	bool m_bIsLoop;
	float m_DurationTime;
	float m_ParticlePlayTime;
};