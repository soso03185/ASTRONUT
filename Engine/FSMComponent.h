#pragma once
#include "Component.h"

class FSM;

/// <summary>
/// FSM�� ������Ʈ�� ���̱� ���� Component
/// FSM ������ ������ ����.
/// GameObject
/// -FSMComponent(!!)
///  -FSM
///   -FSMState
///    -FSMTransition
/// 
/// EventManager�� ���ؼ� ������ �޾ƿ��� �����Ѵ�.
/// </summary>
class FSMComponent
	: public Component
{
public:
	void AttachFSM(std::shared_ptr<FSM> FSM);

	void Initialize() final;
	void Update(float deltaTime) final;

private:
	std::shared_ptr<FSM> m_FSM;
};

