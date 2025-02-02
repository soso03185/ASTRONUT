#pragma once
#include "../Engine/GameObject.h"
#include "../Engine/EventListener.h"

class ParticleComponent;
class Event;

class ShockParticleObjcect
	: public GameObject
	, public EventListener

{
public:
	ShockParticleObjcect();
	~ShockParticleObjcect();

	void PlayParticle();

	void Initialize() override;
	void Update(float deltaTime) override;

	// EventListener��(��) ���� ��ӵ�
	void HandleEvent(Event* event) override;

private:
	// ��� ��ƼŬ
	std::shared_ptr<ParticleComponent> m_ShockParticleComponentOne;
	std::shared_ptr<ParticleComponent> m_ShockParticleComponentTwo;
	std::shared_ptr<ParticleComponent> m_ShockParticleComponentThree;
	std::shared_ptr<ParticleComponent> m_ShockParticleComponentFour;
};

