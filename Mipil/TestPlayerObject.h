#pragma once
#include "../Engine/EventListener.h"
#include "../Engine/GameObject.h"
#include "../Engine/InputManager.h"

#include "PlayerObject.h"

class GridManager;
class PlayerFSM;
class SkeletalMeshComponent;
class FSMComponent;

// �÷��̾� �Է��� ���� �� �����¿�/���/�������� �޴µ�
// �׼� Ÿ���� ���� �� ������, grid::�̰ɷ� ��ĥ����
// ���߿� �ʿ��ϸ� �׼� Ÿ�� ���� �δ°ɷ�
//enum class eActionType
//{
//	WAIT,
//	MOVE,
//	UP,
//
//	END
//};

//enum class eComplexType
//{
//	NOTHING,	// ��ü���� ���� ����
//	PARENT,		// ��ü�� �θ� �÷��̾��� ���
//	CHILD,		// ��ü�� �ڽ� �÷��̾��� ���
//
//	END
//};

class TestPlayerObject
	: public GameObject
	, public EventListener
	, public InputProcesser
{
public:
	TestPlayerObject();
	~TestPlayerObject();

	bool GetHostPlayer() const { return m_bHostPlayer; }
	bool GetTurn() const { return m_bTurn; }
	std::weak_ptr<GridManager> GetGridManager() { return m_gridManager; }

	eComplexType GetComplexType() const { return m_complexed; }

	void SetHostPlayer(bool isHost) { m_bHostPlayer = isHost; }
	void SetTurn(bool isTurn) { m_bTurn = isTurn; }
	void SetComplexType(eComplexType complexed) { m_complexed = complexed; }
	void SetGridManager(std::shared_ptr<GridManager> gridManager) { m_gridManager = gridManager; }

	void Initialize() override;
	void Update(float deltaTime) override;

public:
	void OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker, const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker) override;

	void HandleEvent(Event* event) override;

// ���� ����
public:
	void ProceedAction(Vector3 action);	// �����κ��� �̹� �ൿ �ϳ� �޾��� �� ���� ó��

	void Network_ProceedAction();
	void Network_CheckAction();
	void Network_ChangeTurn();

	void FallToGround();	// FSM State���� ȣ���ϱ� ���� �뵵

private:
	void ComplexCheck();

	void ComplexPlayer(std::shared_ptr<TestPlayerObject> childObj);	// �θ� �÷��̾�� ȣ��
	void UnComplexPlayer(std::shared_ptr<TestPlayerObject> otherObj);

private:
	std::shared_ptr<SkeletalMeshComponent> m_meshComponent;
	std::shared_ptr<PlayerFSM> m_playerFSM;
	std::shared_ptr<GridManager> m_gridManager;

	// ȣ��Ʈ ������ �÷��̾����� �ƴ���
	bool m_bHostPlayer = false;
	// host ������ Ŭ�������� Ȯ��
	bool m_bTurn = false;

	eComplexType m_complexed = eComplexType::NOTHING;

	Vector3 m_action[4] = { grid::Z_UP, grid::Z_UP ,grid::Z_UP ,grid::Z_UP };

	Vector3 m_hostAction[6] = { grid::X_UP, grid::Z_UP ,grid::Y_UP ,grid::X_UP, grid::Y_UP, grid::Z_DOWN };
	Vector3 m_guestAction[6] = { grid::STAY, grid::STAY, grid::STAY, grid::STAY, grid::STAY, grid::STAY };

	int m_index = 0;





};