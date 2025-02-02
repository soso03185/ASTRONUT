#pragma once

class FSM;
class FSMTransition;

/// <summary>
///	�ν��Ͻ�ȭ�� FSMState�� ����
///	��ȯ ���ǵ��� ������ ������, ���� ��ȯ ���ǵ��� üũ�Ѵ�.
///	State�̸��� State_�� �����ϵ��� ����Ѵ�.
/// </summary>
class FSMState
{
public:
	FSMState() = default;
	virtual ~FSMState() = default;


	std::string GetName() { return m_name; }

	void SetName(std::string_view name) { m_name = name; }
	void SetParentFSM(FSM* parentFSM) { m_pParentFSM = parentFSM; }


	void AddTransition(FSMTransition* transition);

	virtual bool CheckTransition();

	virtual void OnEnter() abstract;
	virtual void OnExit() abstract;

protected:
	virtual void DecideFSMType() abstract;

protected:
	FSM* m_pParentFSM;

	std::string m_name;
	std::vector<FSMTransition*> m_transitions;
};