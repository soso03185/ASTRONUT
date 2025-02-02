#pragma once
#include "../Engine/EventListener.h"
#include "../Engine/GameObject.h"
#include "../Engine/InputManager.h"

#include <vector>

class FireParticleObject;
class GridManager;
class SkeletalMeshComponent;
class FSMComponent;
class PointLightComponent;
class GridMovementComponent;
class RigidDynamicComponent;
class UIMeshComponent;
class MaterialManagerComponent;
class PlayerFSM;

enum class eComplexType
{
    NOTHING,	// ��ü���� ���� ����
    PARENT,		// ��ü�� �θ� �÷��̾��� ���
    CHILD,		// ��ü�� �ڽ� �÷��̾��� ���

    END
};

class PlayerObject
    : public GameObject
    , public InputProcesser
    , public EventListener
{
public:
    PlayerObject();
    ~PlayerObject();

    void Initialize() override;
    void Update(float deltaTime) override;

    bool GetHostPlayer() const { return m_bHostPlayer; }
    std::shared_ptr<GridManager> GetGridManager() const { return m_gridManager; }
    bool GetTurn() const { return m_bTurn; }
    bool GetReadyState() const { return m_bReadyState; }
    int GetActionIndex() const { return m_actionIndex; }
    int GetLoadingIndex() const { return m_loadingIndex; }
    Vector3 GetAction(int idx) const { return m_actions[idx]; }
    eComplexType GetComplexType() const { return m_complexed; }
    bool GetWaitingChange() { return m_bWaitingChange; }

	void SetHostPlayer(bool isHost) { m_bHostPlayer = isHost; }
    void SetGridManager(std::shared_ptr<GridManager> gm) { m_gridManager = gm; }
    void SetTurn(bool isTurn) { m_bTurn = isTurn; }
    void SetReadyState(bool isReady) { m_bReadyState = isReady; }
    void SetActionIndex(int idx) { m_actionIndex = idx; }
    void SetLoadingIndex(int idx) { m_loadingIndex = idx; }
    void SetActions(char actions[]);
    void SetComplexType(eComplexType complexed) { m_complexed = complexed; }
    void SetLoadingActionUI();
    void SetExecuteBehavior(bool isExecuteBehavior);
    void SetWaitingChange(bool isWaiting) { m_bWaitingChange = isWaiting; }

    // ����� �� ��ƼŬ ������Ʈ
    void SetFireParticle(std::shared_ptr<FireParticleObject> fireParticle) { m_fireParticleObject = fireParticle; }

// Network
public:
    void Network_SendAction();
    void Network_ChangeTurn();

    char VecToChar(Vector3& vec);
    Vector3 CharToVec(char& buf);

// ���� ����
public:
    void ProceedAction();
    bool CheckActionEnd();
    void ResetIndex();
    void TimeOut();
    void InCreaseIndex() { m_actionIndex++; }
    void ChangeFinishedActionUI();
    void SmaileMaterial(bool isComeback, int comebackTime);
    void CrashMaterial(bool isComeback, int comebackTime);
    void InitPlayer();

    void FallToGround();    // Idle State���� ȣ��
    bool IsIdleState();
    void MovementFinished();
    void AddPart() { m_getParts++; }
    int GetPart() { return m_getParts; }

public:
    void OnInputProcess(
        const Keyboard::State& KeyState,
        const Keyboard::KeyboardStateTracker& KeyTracker,
        const Mouse::State& MouseState,
        const Mouse::ButtonStateTracker& MouseTracker) override;

    void HandleEvent(Event* event) override;

private:
    void ComplexCheck();

    void ComplexPlayer(std::shared_ptr<PlayerObject> childObj);	// �θ� �÷��̾�� ȣ��
    void UnComplexPlayer(std::shared_ptr<PlayerObject> otherObj);   //test������ public

private:
    std::shared_ptr<SkeletalMeshComponent> m_meshComponent;
    std::shared_ptr<FSMComponent> m_fsmComponent;
    std::shared_ptr<PointLightComponent> m_pointLightComponent;
    std::shared_ptr<GridMovementComponent> m_gridMovementComponent;
    std::shared_ptr<RigidDynamicComponent> m_rigidDynamicComponent;

    Matrix uiMatrix;
    std::shared_ptr<UIMeshComponent> m_uiMeshComponent;
    std::shared_ptr<MaterialManagerComponent> m_materialManagerComponent1;
    std::shared_ptr<MaterialManagerComponent> m_materialManagerComponent2;
    std::shared_ptr<MaterialManagerComponent> m_materialManagerComponent3;

    std::shared_ptr<GridManager> m_gridManager;
    std::shared_ptr<PlayerFSM> m_playerFSM;
    std::shared_ptr<FireParticleObject> m_fireParticleObject;

    std::vector<Vector3> m_actions;

    // ȣ��Ʈ ������ �÷��̾����� �ƴ���
    bool m_bHostPlayer = false;
    // host ������ Ŭ�������� Ȯ��
    bool m_bTurn = false;
    // ���� ���� �������� �ƴ���
    bool m_bReadyState = false;

    int m_loadingIndex = 0;
    int m_actionIndex = 0;
    // ��� ���� �Է� �޾��� �� 1�ʵ��� �����̰� �ϱ�
    float m_stayTimer = 0.0f;
    bool m_bstayTimer = false;
    // ��� �����ε� �� �ٲ㵵 �Ǵ� ��� �Ǵ�
    bool m_bChangeTurn = false;
    // ��ü���� ��� ����
    bool m_bWaitingChange = false;

    int m_getParts = 0;

    eComplexType m_complexed = eComplexType::NOTHING;
};