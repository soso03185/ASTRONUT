#pragma once
#include "GridManager.h"

#include "../Engine/World.h"
#include "../Engine/EventListener.h"
#include "../Engine/InputManager.h"

class FireParticleObject;
class PlayerCamera;
class DivisionMap;
class UIPanel;

class PuzzleWorld
	: public World
	, public EventListener
	, public InputProcesser
{
public:
	PuzzleWorld();
	~PuzzleWorld() override;

	void Initialize() override;
	void Update(float deltaTime) override;

	bool GetGoalIn();
	bool GetBanActionInput() const { return m_bBanActionInput; }
	std::shared_ptr<PlayerObject> GetOtherPlayer(PlayerObject* player);

	void SetPlayersPosition(Vector3 hostPos, Vector3 guestPos) { m_playerPositions.first = hostPos; m_playerPositions.second = guestPos; }
	void SetDivisionMapPosition(Vector3 pos) { m_divisionMapPosition = pos; }
	void SetGoalGridPosition(GridPos& goal1, GridPos& goal2) { m_goalGridPos[0] = goal1; m_goalGridPos[1] = goal2; }
	void SetTurnNum(int turn) { m_maxTurnValue = turn; m_numTurn = turn; }
	void SetStageParts(UINT parts) { m_stageParts = parts; }

	// �������� UI�̸� �������� ���� �Լ�
	std::wstring ExtractUIName(std::string uiName, std::string index);
	std::wstring ConvertToActionUI(Vector3 vec, bool actionDone);
	bool CheckRemainingAction();
	bool CheckGoalIn(bool isGoalIn);

	void AddNumParts() { m_numParts++; }
	void SubNumParts();

	void InitMiniMap();
	// �������� ���� �� ���� �ʱ�ȭ
	void InitAll();
	void UIChange();
	std::shared_ptr<UIPanel> GetFadeInOut() { return m_tFadeInOut; }

public:
	void OnEnter() override;
	void OnExit() override;

	void HandleEvent(Event* event) override;

	void OnInputProcess(
		const Keyboard::State& KeyState,
		const Keyboard::KeyboardStateTracker& KeyTracker,
		const Mouse::State& MouseState,
		const Mouse::ButtonStateTracker& MouseTracker) override;

protected:
	//Network
	void Network_ClientsAllReady(char* pData);
	void Network_ClientsAllEnter(char* pData);
	void Network_SetTurn(char* pData);
	void Network_SetUIAction(char* pData);
	void Network_MovementEnd(char* pData);
	void Network_SendAction();
	void NetWork_ActionEnd();

	void ProceedAction(char* pData);

protected:
	// �ൿ�Է� ������ �ΰ���ȭ��������
	bool m_bGameRun = false;
	// Ÿ�̸� ������
	bool m_bTimerRun = false;

	// �÷��̾� �ൿ ����
	bool m_bBanActionInput = false;

	bool m_bShowGameStart = false;
	bool m_bShowStageClear = false; // false; �׽�Ʈ�� ���� true .. �ٽ� false�� �ٲ����
	bool m_bSendStartTurn = false;
	bool m_bStage1PopUp = false;

	float m_timeOut = TIMEOUT;
	float m_showGameStart = SHOW_GAME_START;
	float m_showStageClear = 0.f;

	std::shared_ptr<PlayerCamera> m_Camera;
	std::weak_ptr<PlayerObject> m_hostPlayer;
	std::weak_ptr<PlayerObject> m_guestPlayer;
	std::shared_ptr<FireParticleObject> m_hostFire;
	std::shared_ptr<FireParticleObject> m_guestFire;
	std::shared_ptr<GridManager> m_gridManager;

	std::shared_ptr<DivisionMap> m_divisionMap;

	std::pair<Vector3, Vector3> m_playerPositions;
	Vector3 m_divisionMapPosition;

	UINT m_numParts = 0;
	UINT m_gainedParts = 0;
	UINT m_numTurn = 0;

	GridPos m_goalGridPos[2];

	UINT m_useTurnValue = 0;	// m_numTurn-- �κп��� ++
	UINT m_maxTurnValue = 0;	// SetTurn�� ���ڷ� ������ �� ���

	// �������� ��ü ����
	UINT m_stageParts = 0;

	// ����
	std::shared_ptr<UIPanel> m_dimUI;
	std::shared_ptr<UIPanel> m_tFadeInOut;
	float m_delayTime;
};