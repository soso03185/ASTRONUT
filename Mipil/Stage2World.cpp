#include "pch.h"
#include "Stage2World.h"

#include "PlayerObject.h"
#include "BlockGenerator.h"
#include "PartGenerator.h"
#include "GridManager.h"

#include "DivisionMap.h"
#include "GoalParticleObject.h"
#include "FloorBlockObject.h"

#include "../Engine/CameraComponent.h"
#include "../Engine/GameObject.h"
#include "../Engine/UIFunctorPopUp.h"
#include "../Engine/UIObject.h"
#include "../Engine/UIPanel.h"
#include "../Engine/UIText.h"
#include "../Engine/WorldManager.h"
#include "../Engine/EventManager.h"
#include "../Engine/SoundManager.h"
#include "../Engine/RigidStaticComponent.h"

Stage2World::Stage2World()
	: PuzzleWorld()
{
}

Stage2World::~Stage2World()
{
}

void Stage2World::Initialize()
{
	LOG_CONTENTS("Stage2 Init");

	InitAll();

	std::shared_ptr<BlockGenerator> blockGenerator = std::make_shared<BlockGenerator>(shared_from_this(), "../Resources/Map/stage_2.yaml");
	std::shared_ptr<PartGenerator> partGenerator = std::make_shared<PartGenerator>(shared_from_this(), "../Resources/Map/stage_2.yaml");
	blockGenerator->Generate();
	partGenerator->Generate();
	SetStageParts(m_numParts);

	std::shared_ptr<FloorBlockObject> floorBlock = CreateGameObject<FloorBlockObject>("FloorBlock", eObjectType::TEST).lock();
	floorBlock->SetFilePath("../Resources/FBX/basetile5x6.fbx");
	std::shared_ptr<FloorBlockObject> floorBlock1 = CreateGameObject<FloorBlockObject>("FloorBlock", eObjectType::TEST).lock();
	floorBlock1->SetFilePath("../Resources/FBX/basepan.fbx");

	SetPlayersPosition({ 990.f, 90.f, 270.f }, { 990.f, 90.f, 630.f });
	// �� ����
	SetTurnNum(5);

	// ��ǥ ����
	GridPos goal1 = { 0, 2, 4 };
	GridPos goal2 = { 0, 0, 3 };
	SetGoalGridPosition(goal1, goal2);

	// ��ǥ ���� ��ƼŬ
	std::shared_ptr<GoalParticleObject> GoalParticleObject1 = CreateGameObject<GoalParticleObject>("GoalParticleObject1", eObjectType::PARTICLE).lock();
	std::shared_ptr<GoalParticleObject> GoalParticleObject2 = CreateGameObject<GoalParticleObject>("GoalParticleObject2", eObjectType::PARTICLE).lock();

	SetDivisionMapPosition({ 540.f, 0.f, 360.f });

	// �� ��ü �ڽ� �ݶ��̴�
	auto hollowBox = CreateGameObject<GameObject>("hollowBox", eObjectType::HOLLOWBOX).lock();
	auto hollowBoxRigidBody = hollowBox->CreateComponent<RigidStaticComponent>("hollowBox_Rigidbody");
	auto hollowBoxSceneComponent = hollowBox->CreateComponent<SceneComponent>("hollowBox_SceneComponent");
	hollowBox->SetRootComponent(hollowBoxSceneComponent.lock());
	hollowBoxRigidBody.lock()->CreateHollowBox(1080.f, 720.f, 900.f, { 0.f, 0.f, 0.f });

	// �̴ϸ� UI(Ȧ�α׷� �г� �ڽ�)
	auto MiniMapUI = m_UIManager->CreateUI<UIPanel>(L"Hologram_MiniMapUI");
	MiniMapUI->SetTexturePath(L"../Resources/Textures/1.png");
	MiniMapUI->SetPosition({ 479.f, 113.f });
	MiniMapUI->SetSize({ 494.f, 413.f });
	MiniMapUI->SetVisible(true);

	// �̴ϸʿ� �ִ� Player(�̴ϸ� UI �г� �ڽ�)
	auto MiniMapPlayerUI = m_UIManager->CreateUI<UIPanel>(L"MiniMap_PlayerUI");
	MiniMapPlayerUI->SetTexturePath(L"../Resources/Textures/Input_player token.png");
	MiniMapPlayerUI->SetSize({ 50.f, 50.f });
	MiniMapPlayerUI->SetVisible(true);

	// �̴ϸʿ� �ִ� friend(�̴ϸ� UI �г� �ڽ�)
	auto MiniMapFriendUI = m_UIManager->CreateUI<UIPanel>(L"MiniMap_FriendUI");
	MiniMapFriendUI->SetTexturePath(L"../Resources/Textures/Input_Friends token.png");
	MiniMapFriendUI->SetSize({ 50.f, 50.f });
	MiniMapFriendUI->SetVisible(true);
	MiniMapUI->AddChildren(MiniMapPlayerUI, MiniMapFriendUI);

	// �̴ϸʿ� �ִ� target(�̴ϸ� UI �г� �ڽ�)
	for (int i = 0; i < m_numParts; i++)
	{
		std::string str = "MiniMap_TargetUI";
		str += std::to_string(i);
		auto targetUI = m_UIManager->CreateUI<UIPanel>(std::wstring().assign(str.begin(), str.end()));
		targetUI->SetTexturePath(L"../Resources/Textures/Input_target token.png");
		targetUI->SetSize({ 50.f, 50.f });
		targetUI->SetVisible(true);
		MiniMapUI->AddChildren(targetUI);
	}

	PuzzleWorld::Initialize();

	floorBlock->SetPosition({ 540.f, -60.f, 450.f });		// �ٴ� ��� ��ġ ����
	floorBlock->SetRotation({ 0.f, 90.f, 0.f });		// �ٴ� ��� ȸ�� ����
	GoalParticleObject1->SetGoalPosition(goal1.x, goal1.y, goal1.z);
	GoalParticleObject2->SetGoalPosition(goal2.x, goal2.y, goal2.z);

	// Ȧ�α׷��� �̴ϸ� �߰����ֱ�
	auto hologramPanelUI = m_UIManager->GetUIObject<UIPanel>(L"Hologram_PanelUI");
	hologramPanelUI.lock()->AddChildren(MiniMapUI);

	// �������� Ŭ���� �г� ���� �̹���(�������� 2�� 45_2)
	auto stageClear_Rocket = m_UIManager->GetUIObject<UIPanel>(L"StageClear_Rocket");
	stageClear_Rocket.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/45_2.png");

	// �������� Ŭ���� �г� �ۼ�Ʈ(�������� 2�� 45%)
	auto rocketProcessPercent = m_UIManager->GetUIObject<UIText>(L"StageClear_RocketProcessPercent");
	rocketProcessPercent.lock()->SetText(L"45%");

	/// GridManager
	m_gridManager = std::make_shared<GridManager>();
	m_gridManager->CreateMap(6, 4, 5, 0.f, 0.f, 0.f, 180.f);
	m_gridManager->SetMapState(GetGameObjects(eObjectType::LEVEL));
	m_gridManager->SetMapState(GetGameObjects(eObjectType::PART));
	m_gridManager->SetMapState(GetGameObjects(eObjectType::PLAYER));

	// �̴ϸ� ������ �ʱ�ȭ
	InitMiniMap();

	m_hostPlayer.lock()->SetGridManager(m_gridManager);
	m_guestPlayer.lock()->SetGridManager(m_gridManager);

	// stage1������ HostPlayer���� ���ֱ�
	m_hostPlayer.lock()->SetTurn(true);
}

void Stage2World::Update(float deltaTime)
{
	// 3�ʵ��� �������� Ŭ���� �˾� �����ֱ�
	if (m_bShowStageClear)
	{
		static bool once = true;
		if (once)
		{
			once = false;
			SoundManager::GetInstance()->PlaySound("../Resources/Sound/Effect/Stage_Clear.mp3");
		}

		auto StageClearPopUpUI_A = m_UIManager->GetUIObject<UIFunctorPopUp>(L"PopUpUI_StageClear");
		StageClearPopUpUI_A.lock()->SetAllVisible(true);

		m_dimUI->GetUIInstance().lock()->SetUseDim(true);

		m_showStageClear += deltaTime;
		m_bGameRun = false;
		if (m_showStageClear >= SHOW_STAGE_CLEAR)
		{
			once = true;
			m_showStageClear = 0.0f;
			m_bShowStageClear = false;
			StageClearPopUpUI_A.lock()->SetAllVisible(false);
			// todo ä�� :changeworld
			EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STAGE3);
		}
	}

	PuzzleWorld::Update(deltaTime);
}

void Stage2World::OnEnter()
{
	PuzzleWorld::OnEnter();
}

void Stage2World::OnExit()
{
	PuzzleWorld::OnExit();
}