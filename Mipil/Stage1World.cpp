#include "pch.h"
#include "Stage1World.h"

#include "PlayerObject.h"
#include "BlockGenerator.h"
#include "PartGenerator.h"
#include "GoalParticleObject.h"
#include "FloorBlockObject.h"
#include "DivisionMap.h"
#include "BlockPOD.h"
#include "UITutorialNextClick.h"
#include "UITutorialPrevClick.h"
#include "UITutorialExitClick.h"

#include "../Engine/CameraComponent.h"
#include "../Engine/ControllerComponent.h"
#include "../Engine/FPSCameraController.h"

#include "../Engine/CameraComponent.h"
#include "../Engine/GameObject.h"
#include "../Engine/UIObject.h"
#include "../Engine/UIPanel.h"
#include "../Engine/UIText.h"
#include "../Engine/UIButton.h"
#include "../Engine/UIFunctorPopUp.h"
#include "../Engine/WorldManager.h"
#include "../Engine/RigidStaticComponent.h"
#include "../Engine/SoundManager.h"
#include "../Engine/UIFunctorPopUp.h"
#include "../Engine/EventManager.h"

Stage1World::Stage1World()
	: PuzzleWorld()
{
}

Stage1World::~Stage1World()
{
}

void Stage1World::Initialize()
{
	LOG_CONTENTS("Stage1 Init");

	// �������� �� ���� ���� �ʱ�ȭ ���ֱ�
	InitAll(); 
	m_bStage1PopUp = true;
	m_GuideNumber = 0;

	//std::shared_ptr<BlockGenerator> blockGenerator = std::make_shared<BlockGenerator>(shared_from_this(), "../Resources/Map/block_test2.yaml");
	std::shared_ptr<BlockGenerator> blockGenerator = std::make_shared<BlockGenerator>(shared_from_this(), "../Resources/Map/stage_1.yaml");
	std::shared_ptr<PartGenerator> partGenerator = std::make_shared<PartGenerator>(shared_from_this(), "../Resources/Map/stage_1.yaml");
	blockGenerator->Generate();
	partGenerator->Generate();
	SetStageParts(m_numParts);

	std::shared_ptr<FloorBlockObject> floorBlock = CreateGameObject<FloorBlockObject>("FloorBlock", eObjectType::TEST).lock();
	floorBlock->SetFilePath("../Resources/FBX/basetile5x6.fbx");
	std::shared_ptr<FloorBlockObject> floorBlock1 = CreateGameObject<FloorBlockObject>("FloorBlock", eObjectType::TEST).lock();
	floorBlock1->SetFilePath("../Resources/FBX/basepan.fbx");

	SetPlayersPosition({ 270.f, 450.f, 810.f }, { 90.f, 450.f, 630.f });
	 
	// �� ����
	SetTurnNum(4);

	// ��ǥ ����
	GridPos goal1 = { 4, 1, 0 };
	GridPos goal2 = { 5, 2, 0 };
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
	MiniMapUI->SetTexturePath(L"../Resources/Textures/5.png");
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
	for (int i = 0; i < m_stageParts; i++)
	{
		std::string str = "MiniMap_TargetUI";
		str += std::to_string(i);
		auto targetUI = m_UIManager->CreateUI<UIPanel>(std::wstring().assign(str.begin(), str.end()));
		targetUI->SetTexturePath(L"../Resources/Textures/Input_target token.png");
		targetUI->SetSize({ 50.f, 50.f });
		targetUI->SetVisible(true);
		MiniMapUI->AddChildren(targetUI);
	}

	// ���ӹ�� �˾� UI
	auto GuidePanelUI = m_UIManager->CreateUI<UIPanel>(L"PopUp_GuidePanel");
	GuidePanelUI->SetTexturePath(L"../Resources/Textures/Input_p_guide1_wPhoto.png");
	GuidePanelUI->SetPosition({ 0.f, 0.f });
	GuidePanelUI->SetSize({ 1340.f, 769.f });

	// to ����.. ����ٰ� functor �����..
	auto GuideNextFunction = std::make_shared<UITutorialNextClick>();
	auto GuidePrevFunction = std::make_shared<UITutorialPrevClick>();
	auto GuideExitFunction = std::make_shared<UITutorialExitClick>();
	GuideNextFunction->SetCount(&m_GuideNumber);
	GuidePrevFunction->SetCount(&m_GuideNumber);
	GuideExitFunction->SetStage1Guide(&m_bStage1PopUp);
	
	auto GuideNextButtonUI = m_UIManager->CreateUI<UIButton>(L"PopUp_GuideNextButton");
	GuideNextButtonUI->SetTexturePath(L"../Resources/Textures/Input_p_btn_guideNext.png");
	GuideNextButtonUI->SetPosition({ 590.f, 0.f });
	GuideNextButtonUI->SetSize({ 38.f * 1.2f, 78.f * 1.2f });
	GuideNextButtonUI->SetVisible(false);
	GuideNextButtonUI->SetMouseClickFunctor(GuideNextFunction);

	auto GuidePrevButtonUI = m_UIManager->CreateUI<UIButton>(L"PopUp_GuidePrevButton");
	GuidePrevButtonUI->SetTexturePath(L"../Resources/Textures/Input_p_btn_guidePrev.png");
	GuidePrevButtonUI->SetPosition({ -598.f, 0.f });
	GuidePrevButtonUI->SetSize({ 38.f * 1.2f, 78.f * 1.2f });
	GuidePrevButtonUI->SetVisible(false);
	GuidePrevButtonUI->SetMouseClickFunctor(GuidePrevFunction);

	auto GuideExitButtonUI = m_UIManager->CreateUI<UIButton>(L"PopUp_GuideExitButton");
	GuideExitButtonUI->SetTexturePath(L"../Resources/Textures/Input_p_guide3_btn.png");
	GuideExitButtonUI->SetPosition({ 479.f, -225.f });
	GuideExitButtonUI->SetSize({ 186.f * 1.2f, 72.f * 1.2f });
	GuideExitButtonUI->SetVisible(false);
	GuideExitButtonUI->SetMouseClickFunctor(GuideExitFunction);

	GuidePanelUI->AddChildren(GuideNextButtonUI, GuidePrevButtonUI, GuideExitButtonUI);
	GuidePanelUI->SetAllVisible(false);

	PuzzleWorld::Initialize();

	floorBlock->SetPosition({ 540.f, -60.f, 450.f });		// �ٴ� ��� ��ġ ����
	floorBlock->SetRotation({ 0.f, 90.f, 0.f });		// �ٴ� ��� ȸ�� ����
	GoalParticleObject1->SetGoalPosition(goal1.x, goal1.y, goal1.z );
	GoalParticleObject2->SetGoalPosition(goal2.x, goal2.y, goal2.z );

	// Ȧ�α׷��� �̴ϸ� �߰����ֱ�
	auto hologramPanelUI = m_UIManager->GetUIObject<UIPanel>(L"Hologram_PanelUI");
	hologramPanelUI.lock()->AddChildren(MiniMapUI);

	// �������� Ŭ���� �г� ���� �̹���(�������� 1�� 21_2)
	auto stageClear_Rocket = m_UIManager->GetUIObject<UIPanel>(L"StageClear_Rocket");
	stageClear_Rocket.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/21_2.png");
	
	// �������� Ŭ���� �г� �ۼ�Ʈ(�������� 1�� 21%)
	auto rocketProcessPercent = m_UIManager->GetUIObject<UIText>(L"StageClear_RocketProcessPercent");
	rocketProcessPercent.lock()->SetText(L"21%");

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

	m_hostPlayer.lock()->SetTurn(true);
}

void Stage1World::Update(float deltaTime)
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
			EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STAGE2);
		}
	}

	PuzzleWorld::Update(deltaTime);

	// to ����.. ����ٰ� �ϸ� ��..
	if (m_bStage1PopUp)
	{
		// Ȧ�α׷� ����
		{
			auto ui = m_UIManager->GetUIObject<UIPanel>(L"Hologram_PanelUI");
			if (ui.lock()->GetVisible())
			{
				ui.lock()->SetAllVisible(false);
			}
		}
		// ����� ����
		{
			auto ui = m_UIManager->GetUIObject<UIPanel>(L"InputAction_PanelUI");
			if (ui.lock()->GetVisible())
			{
				ui.lock()->SetAllVisible(false);
			}
		}
		// Ȧ�α׷� �ִϸ��̼� ����
		{
			auto ui = m_UIManager->GetUIObject<UIPanel>(L"Input_AnimALLUI");
			if (ui.lock()->GetVisible())
			{
				ui.lock()->SetAllVisible(false);
			}
		}

		auto guidePanel = m_UIManager->GetUIObject<UIPanel>(L"PopUp_GuidePanel");
		guidePanel.lock()->SetAllVisible(true);

		m_dimUI->GetUIInstance().lock()->SetUseDim(false);

		if (m_GuideNumber <= 0)
		{
			auto GuideNextButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuideNextButton");
			auto GuidePrevButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuidePrevButton");
			auto GuideExitButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuideExitButton");
			auto GuidePanelUI = m_UIManager->GetUIObject<UIPanel>(L"PopUp_GuidePanel");
			GuidePanelUI.lock()->SetTexturePath(L"../Resources/Textures/Input_p_guide1_wPhoto.png");
			GuidePanelUI.lock()->Initialize();

			GuideNextButtonUI.lock()->SetVisible(true);
			GuidePrevButtonUI.lock()->SetVisible(false);
			GuideExitButtonUI.lock()->SetVisible(false);
		}
		else if (m_GuideNumber == 1)
		{
			auto GuideNextButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuideNextButton");
			auto GuidePrevButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuidePrevButton");
			auto GuideExitButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuideExitButton");
			auto GuidePanelUI = m_UIManager->GetUIObject<UIPanel>(L"PopUp_GuidePanel");
			GuidePanelUI.lock()->SetTexturePath(L"../Resources/Textures/Input_p_guide2_wPhoto.png");
			GuidePanelUI.lock()->Initialize();

			GuideNextButtonUI.lock()->SetVisible(true);
			GuidePrevButtonUI.lock()->SetVisible(true);
			GuideExitButtonUI.lock()->SetVisible(false);
		}
		else if (m_GuideNumber >= 2)
		{
			auto GuideNextButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuideNextButton");
			auto GuidePrevButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuidePrevButton");
			auto GuideExitButtonUI = m_UIManager->GetUIObject<UIButton>(L"PopUp_GuideExitButton");
			auto GuidePanelUI = m_UIManager->GetUIObject<UIPanel>(L"PopUp_GuidePanel");
			GuidePanelUI.lock()->SetTexturePath(L"../Resources/Textures/Input_p_guide3_wPhoto.png");
			GuidePanelUI.lock()->Initialize();

			GuideNextButtonUI.lock()->SetVisible(false);
			GuidePrevButtonUI.lock()->SetVisible(true);
			GuideExitButtonUI.lock()->SetVisible(true);
		}
	}
	//else if (!m_bStage1PopUp && !m_bShowGameStart)
	//{
	//	auto GuidePanelUI = m_UIManager->GetUIObject<UIPanel>(L"PopUp_GuidePanel");
	//	GuidePanelUI.lock()->SetAllVisible(false);
	//	// todo ä��: ����
	//	UIChange();
	//}
}

void Stage1World::OnEnter()
{
	PuzzleWorld::OnEnter();
	//GetFadeInOut()->GetUIInstance().lock()->FadeInStart();
}

void Stage1World::OnExit()
{
	PuzzleWorld::OnExit();
}
