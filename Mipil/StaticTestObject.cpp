#include "pch.h"
#include "StaticTestObject.h"

#include "../Engine/StaticMeshComponent.h"
#include "../Engine/ParticleComponent.h"

void StaticTestObject::Initialize()
{
	m_meshComponent = CreateComponent<StaticMeshComponent>("StaticMeshComponent");
	//m_meshComponent.lock()->SetFilePath("../Resources/FBX/zeldaPosed001.fbx");
	//m_meshComponent.lock()->SetFilePath("../Resources/FBX/cerberus2.fbx");
	m_meshComponent.lock()->SetFilePath("../Resources/FBX/sphere_alpha3.fbx");
	SetRootComponent(m_meshComponent.lock());

	// ��ƼŬ ���� ����
	{
		m_ParticleComponent1 = CreateComponent<ParticleComponent>("ParticleComponent");
		ParticleComponentInfo info;
		info.Acceleration = Vector3(0.f, -100.f, 0.f);		// ���ӵ�
		info.CreatePerSecond = 10.f;						// 1�ʴ� ���� ����
		info.CreateTime = 5.f;								// ���� �ð� ( �̱��� )
		info.GenerationRange = Vector3(10.f, 0.f, 10.f);	// �ʱ� ��ġ ����
		info.InitialSpeed = Vector3(0.f, -2.f, 0.f);		// �ʱ� �ӵ�
		info.Size = Vector2(30.f, 30.f);					// ��ƼŬ ũ��
		info.LifeTime = 1.f;								// ���� �ð�
		info.m_ParticleType = eParticleType::FIRE;
		info.m_ShaderPath = L"../Shader/Fire.hlsl";
		info.m_TexturePath = L"../Resources/Textures/Stylized_Fire_particle_texture_1.png";
		m_ParticleComponent1.lock()->SetVisible(true);
		m_ParticleComponent1.lock()->Setting(info);
		m_ParticleComponent1.lock()->SetLocalPosition(Vector3(200.f, 0.f, 200.f));
	}
	{
		m_ParticleComponent2 = CreateComponent<ParticleComponent>("ParticleComponent");
		ParticleComponentInfo info;
		info.Acceleration = Vector3(0.f, -100.f, 0.f);		// ���ӵ�
		info.CreatePerSecond = 10.f;						// 1�ʴ� ���� ����
		info.CreateTime = 5.f;								// ���� �ð� ( �̱��� )
		info.GenerationRange = Vector3(10.f, 0.f, 10.f);	// �ʱ� ��ġ ����
		info.InitialSpeed = Vector3(0.f, -2.f, 0.f);		// �ʱ� �ӵ�
		info.Size = Vector2(30.f, 30.f);					// ��ƼŬ ũ��
		info.LifeTime = 1.f;								// ���� �ð�
		info.m_ParticleType = eParticleType::FIRE;
		info.m_ShaderPath = L"../Shader/Fire.hlsl";
		info.m_TexturePath = L"../Resources/Textures/Stylized_Fire_particle_texture_2.png";
		m_ParticleComponent2.lock()->SetVisible(true);
		m_ParticleComponent2.lock()->Setting(info);
		m_ParticleComponent2.lock()->SetLocalPosition(Vector3(200.f, 0.f, 200.f));
	}
	{
		m_ParticleComponent3 = CreateComponent<ParticleComponent>("ParticleComponent");
		ParticleComponentInfo info;
		info.Acceleration = Vector3(0.f, -100.f, 0.f);		// ���ӵ�
		info.CreatePerSecond = 10.f;						// 1�ʴ� ���� ����
		info.CreateTime = 5.f;								// ���� �ð� ( �̱��� )
		info.GenerationRange = Vector3(10.f, 0.f, 10.f);	// �ʱ� ��ġ ����
		info.InitialSpeed = Vector3(0.f, -2.f, 0.f);		// �ʱ� �ӵ�
		info.Size = Vector2(30.f, 30.f);					// ��ƼŬ ũ��
		info.LifeTime = 1.f;								// ���� �ð�
		info.m_ParticleType = eParticleType::FIRE;
		info.m_ShaderPath = L"../Shader/Fire.hlsl";
		info.m_TexturePath = L"../Resources/Textures/Stylized_Fire_particle_texture_3.png";
		m_ParticleComponent3.lock()->SetVisible(true);
		m_ParticleComponent3.lock()->Setting(info);
		m_ParticleComponent3.lock()->SetLocalPosition(Vector3(200.f, 0.f, 200.f));
	}

	GameObject::Initialize();
}

void StaticTestObject::Update(float deltaTime)
{
	GameObject::Update(deltaTime);
}
