#pragma once

class GameObject;
class FSMState;

/// <summary>
/// ������Ʈ�� ���� �ν��Ͻ�ȭ�� FSM�� ����
///	��� ���� �߿��� ���� ���¿� ���� ���¸� ������ ��ȯ, update ���ش�.
/// </summary>
class FSM
{
public:
	FSM() = default;
	virtual ~FSM() = default;

	void SetNextState(std::string_view name) { m_pNextState = m_stateMap.find(std::string(name))->second; }
	void SetDefaultState(std::string_view name);

	std::string& GetAnimationName(std::string_view stateName);
	std::string& GetMaterialName(std::string_view stateName);

	void AddState(std::string_view name, FSMState* state);
	void AddAnimation(std::string_view stateName, std::string_view animationName);
	FSMState* FindState(std::string name) const { return m_stateMap.find(name)->second; }

	void Update(float deltaTime);

protected:
	std::unordered_map<std::string, FSMState*> m_stateMap;
	std::unordered_map<std::string, std::string> m_stateAnimationMap;
	std::unordered_map<std::string, std::string> m_stateMaterialMap;

	FSMState* m_pCurState;
	FSMState* m_pNextState;
};