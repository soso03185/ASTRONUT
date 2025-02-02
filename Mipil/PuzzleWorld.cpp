#include "pch.h"
#include "PuzzleWorld.h"
#include "Stage1World.h"
#include "PlayerObject.h"
#include "ShockParticleObjcect.h"
#include "FireParticleObject.h"
#include "CubeMapObject.h"
#include "GridMovementComponent.h"
#include "PlayerCamera.h"
#include "DivisionMap.h"
#include "UIReadyClick.h"
#include "UINoClick.h"
#include "UIReadyClickPopUpFunctor.h"

#include "../Engine/CameraComponent.h"
#include "../Engine/ControllerComponent.h"
#include "../Engine/GameObject.h"
#include "../Engine/UIButton.h"
#include "../Engine/UIFunctorPopUp.h"
#include "../Engine/UIObject.h"
#include "../Engine/UIPanel.h"
#include "../Engine/UIText.h"
#include "../Engine/WorldManager.h"
#include "../Engine/RigidStaticComponent.h"
#include "../Engine/SoundManager.h"

#include <iomanip>   // std::setprecision
#include <sstream>   // std::ostringstream
#include <locale>
#include <format>

#include "../Engine/Event.h"
#include "../Engine/EventManager.h"

PuzzleWorld::PuzzleWorld()
{
	EventManager::GetInstance()->RegisterListener(eEventType::DELETE_OBJECT, this);
	EventManager::GetInstance()->RegisterListener(eEventType::CHANGE_WORLD, this);
}

PuzzleWorld::~PuzzleWorld()
{
	EventManager::GetInstance()->UnregisterListener(eEventType::DELETE_OBJECT, this);
	EventManager::GetInstance()->UnregisterListener(eEventType::CHANGE_WORLD, this);
}

void PuzzleWorld::Initialize()
{
	WorldManager::GetInstance()->RegisterHandler(S2C_IS_ALL_ENTER, std::bind(&PuzzleWorld::Network_ClientsAllEnter, this, std::placeholders::_1));
	WorldManager::GetInstance()->RegisterHandler(S2C_IS_ALL_READY, std::bind(&PuzzleWorld::Network_ClientsAllReady, this, std::placeholders::_1));
	WorldManager::GetInstance()->RegisterHandler(S2C_START_TURN, std::bind(&PuzzleWorld::ProceedAction, this, std::placeholders::_1));
	WorldManager::GetInstance()->RegisterHandler(S2C_SET_TURN, std::bind(&PuzzleWorld::Network_SetTurn, this, std::placeholders::_1));
	WorldManager::GetInstance()->RegisterHandler(S2C_BROADCAST_ACTION, std::bind(&PuzzleWorld::Network_SetUIAction, this, std::placeholders::_1));
	WorldManager::GetInstance()->RegisterHandler(S2C_MOVEMENT_END, std::bind(&PuzzleWorld::Network_MovementEnd, this, std::placeholders::_1));

	SoundManager::GetInstance()->StopAllSound();
	SoundManager::GetInstance()->LoadSound("../Resources/Sound/Effect/Timer_Over.mp3");
	SoundManager::GetInstance()->LoadSound("../Resources/Sound/BGM/BGM_InGame.mp3", FMOD_LOOP_NORMAL);
	SoundManager::GetInstance()->PlaySound("../Resources/Sound/BGM/BGM_InGame.mp3");
	SoundManager::GetInstance()->LoadSound("../Resources/Sound/Effect/Stage_Clear.mp3");
	SoundManager::GetInstance()->LoadSound("../Resources/Sound/Effect/Stage_Gameover.mp3");
	// ���� �ٲ� ������ ���� �ʱ�ȭ(������������ ���ֱ�)
	m_Camera = CreateGameObject<PlayerCamera>("Camera", eObjectType::CAMERA).lock();

	m_divisionMap = std::make_shared<DivisionMap>(this);
	m_divisionMap->SetDeferredPosition(m_divisionMapPosition);
	m_divisionMap->SetMaps();
	m_divisionMap->SetCamera(m_Camera.get());

	// �÷��̾� �ʱ�ȭ
	m_hostPlayer = CreateGameObject<PlayerObject>("HostPlayer", eObjectType::PLAYER);
	m_guestPlayer = CreateGameObject<PlayerObject>("GuestPlayer", eObjectType::PLAYER);
	std::shared_ptr<PlayerObject> hostPlayer = m_hostPlayer.lock();
	std::shared_ptr<PlayerObject> guestPlayer = m_guestPlayer.lock();
	hostPlayer->SetHostPlayer(true);
	guestPlayer->SetHostPlayer(false);
	hostPlayer->SetDeferredPosition({ m_playerPositions.first.x, m_playerPositions.first.y + BASE_HEIGHT, m_playerPositions.first.z });
	guestPlayer->SetDeferredPosition({ m_playerPositions.second.x, m_playerPositions.second.y + BASE_HEIGHT, m_playerPositions.second.z });

	if (WorldManager::GetInstance()->IsHostServer())
		m_Camera->SetSubject(m_hostPlayer);
	else
		m_Camera->SetSubject(m_guestPlayer);

	m_hostFire = CreateGameObject<FireParticleObject>("host's fire", eObjectType::PARTICLE).lock();
	m_hostFire->SetParentObject(hostPlayer);
	//m_hostFire->SetDeferredPosition({ 0.f, -100.f, 0.f });
	hostPlayer->SetFireParticle(m_hostFire);

	m_guestFire = CreateGameObject<FireParticleObject>("guest's fire", eObjectType::PARTICLE).lock();
	m_guestFire->SetParentObject(guestPlayer);
	//m_guestFire->SetDeferredPosition({ 0.f, -100.f, 0.f });
	guestPlayer->SetFireParticle(m_guestFire);


	auto plane = CreateGameObject<GameObject>("Plane", eObjectType::PLANE).lock();
	auto planeRigidBody = plane->CreateComponent<RigidStaticComponent>("Plane_Rigidbody");
	auto planeSceneComponent = plane->CreateComponent<SceneComponent>("Plane_SceneComponent");
	plane->SetRootComponent(planeSceneComponent.lock());
	planeRigidBody.lock()->CreatePlane(0.f, 1.f, 0.f, 0.f, { 0.5f, 0.5f, 0.6f });

	auto shockParticleObjcect = CreateGameObject<ShockParticleObjcect>("Shock_Particle_Objcect", eObjectType::PARTICLE).lock();

	// CubeMap
	CreateGameObject<CubeMapObject>("CubeMap", eObjectType::TEST);

	// dim
	auto dimUI = m_UIManager->CreateUI<UIPanel>(L"dimUI");
	dimUI->SetSize({ 1920.f, 1080.f });
	dimUI->SetPosition({ 0.f, 0.f });
	dimUI->SetScale({ 1.0f, 1.0f });
	m_dimUI = dimUI;

	// FadeInOut
	auto FadeInTestUI = m_UIManager->CreateUI<UIPanel>(L"FadeInTestUI");
	FadeInTestUI->SetSize({ 1920.f, 1080.f });
	FadeInTestUI->SetPosition({ 0.f, 0.f });
	FadeInTestUI->SetScale({ 1.0f, 1.0f });
	m_tFadeInOut = FadeInTestUI;

	/// UI
	// �ൿ�Է� �г�
	auto InputActionUI = m_UIManager->CreateUI<UIPanel>(L"InputAction_PanelUI");
	InputActionUI->SetTexturePath(L"../Resources/Textures/Input_bg_instrument panel.png");
	InputActionUI->SetSize({ 1920.f, 1080.f });
	InputActionUI->SetPosition({ 0.f,  0.f });
	InputActionUI->SetScale({ 1.f, 1.f });
	InputActionUI->SetVisible(true);

	// ���� ��ư ���Ͷ� UI(�ൿ�Է� �г� �ڽ�)
	auto ReadyButtonUI = m_UIManager->CreateUI<UIButton>(L"InputAction_ReadyButtonUI");
	ReadyButtonUI->SetTexturePath(L"../Resources/Textures/Input_ReadyBtn inactive.png");
	ReadyButtonUI->SetPosition({ 533.f, -349.f });
	ReadyButtonUI->SetSize({ 114.f, 103.f });
	ReadyButtonUI->SetVisible(true);

	// ���� �������� UI(�ൿ�Է� �г� �ڽ�)
	auto FirstTurnActiveUI = m_UIManager->CreateUI<UIPanel>(L"InputAction_FirstTurnActiveUI");
	if (WorldManager::GetInstance()->IsHostServer())
		FirstTurnActiveUI->SetTexturePath(L"../Resources/Textures/Input_First Lamp active.png");
	else
		FirstTurnActiveUI->SetTexturePath(L"../Resources/Textures/Input_First Lamp2 active.png");
	FirstTurnActiveUI->SetPosition({ -639.5f, -257.f });
	FirstTurnActiveUI->SetSize({ 401.f, 54.f });
	FirstTurnActiveUI->SetVisible(true);

	InputActionUI->SetAllVisible(true);
	// ���� �Է� �� �ε��� �ൿ 4���� �г�(�ൿ�Է� �г� �ڽ�)
	auto LoadingActionPanelUI = m_UIManager->CreateUI<UIPanel>(L"InputAction_LoadingActionPanelUI");
	LoadingActionPanelUI->SetTexturePath(L"../Resources/Textures/Input_Waiting for call_input.png");
	LoadingActionPanelUI->SetSize({ 644.f, 178.f });
	LoadingActionPanelUI->SetPosition({ 0.f,  -100.f });
	LoadingActionPanelUI->SetScale({ 1.f, 1.f });
	LoadingActionPanelUI->SetVisible(false);

	// ���� �Է� �� �ε��� �ൿ 4���� UI( ���� �Է� �� �ε� �г� �ڽ�)
	auto LoadingAction0UI = m_UIManager->CreateUI<UIPanel>(L"LoadingPanel_ActionUI0");
	LoadingAction0UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	LoadingAction0UI->SetPosition({ -216.f, 0.f });
	LoadingAction0UI->SetSize({ 94.f, 94.f });
	LoadingAction0UI->SetVisible(false);
	// ���� �Է� �� �ε��� �ൿ 4���� UI( ���� �Է� �� �ε� �г� �ڽ�)
	auto LoadingAction1UI = m_UIManager->CreateUI<UIPanel>(L"LoadingPanel_ActionUI1");
	LoadingAction1UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	LoadingAction1UI->SetPosition({ -71.f, 0.f });
	LoadingAction1UI->SetSize({ 94.f, 94.f });
	LoadingAction1UI->SetVisible(false);
	// ���� �Է� �� �ε��� �ൿ 4���� UI( ���� �Է� �� �ε� �г� �ڽ�)
	auto LoadingAction2UI = m_UIManager->CreateUI<UIPanel>(L"LoadingPanel_ActionUI2");
	LoadingAction2UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	LoadingAction2UI->SetPosition({ 75.f, 0.f });
	LoadingAction2UI->SetSize({ 94.f, 94.f });
	LoadingAction2UI->SetVisible(false);
	// ���� �Է� �� �ε��� �ൿ 4���� UI( ���� �Է� �� �ε� �г� �ڽ�)
	auto LoadingAction3UI = m_UIManager->CreateUI<UIPanel>(L"LoadingPanel_ActionUI3");
	LoadingAction3UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	LoadingAction3UI->SetPosition({ 220.f, 0.f });
	LoadingAction3UI->SetSize({ 94.f, 94.f });
	LoadingAction3UI->SetVisible(false);
	LoadingActionPanelUI->AddChildren(LoadingAction0UI, LoadingAction1UI, LoadingAction2UI, LoadingAction3UI);
	InputActionUI->AddChildren(FirstTurnActiveUI, ReadyButtonUI, LoadingActionPanelUI);

	// Ȧ�α׷� �г�
	auto HologramPanelUI = m_UIManager->CreateUI<UIPanel>(L"Hologram_PanelUI");
	HologramPanelUI->SetTexturePath(L"../Resources/Textures/Input_bg_hologram.png");
	HologramPanelUI->SetSize({ 1920.f, 1080.f });
	HologramPanelUI->SetPosition({ 0.f,  0.f });
	HologramPanelUI->SetScale({ 1.f, 1.f });
	HologramPanelUI->SetVisible(true);

	// Ÿ�̸� UI( Ȧ�α׷� �г� �ڽ�)
	auto TimerUI = m_UIManager->CreateUI<UIText>(L"Hologram_TimerUI");
	TimerUI->SetPosition({ -58.f, 198.f });
	TimerUI->SetFont(L"���� �߳�ü ���");
	TimerUI->SetFontSize(50.f);
	TimerUI->SetColor(0xf96694);
	std::wstringstream wss;
	wss << std::fixed << std::setprecision(0) << 0.0f;
	// wstring���� ��ȯ
	std::wstring result = wss.str();
	TimerUI->SetText(result);
	TimerUI->SetVisible(true);

	// ���� �ϼ� UI(Ȧ�α׷� �г� �ڽ�)
	auto InputRemainedTurnUI = m_UIManager->CreateUI<UIText>(L"Hologram_RemainedTurnUI");
	InputRemainedTurnUI->SetPosition({ 751.f, 412.f });
	InputRemainedTurnUI->SetFont(L"KoPubWold����ü Bold");
	InputRemainedTurnUI->SetFontSize(20.f);
	InputRemainedTurnUI->SetColor(0x56b0e1);
	InputRemainedTurnUI->SetText(std::to_wstring(m_numTurn - 1));
	InputRemainedTurnUI->SetVisible(true);

	// �ൿ�Է¿��� �ൿ 4���� �Է¹޴� UI(Ȧ�α׷� �г� �ڽ�)
	auto Action0UI = m_UIManager->CreateUI<UIPanel>(L"Hologram_UIAction0");
	Action0UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	Action0UI->SetPosition({ -681.f, -86.f });
	Action0UI->SetSize({ 94.f, 94.f });
	Action0UI->SetVisible(false);
	// �ൿ�Է¿��� �ൿ 4���� �Է¹޴� UI(Ȧ�α׷� �г� �ڽ�)
	auto Action1UI = m_UIManager->CreateUI<UIPanel>(L"Hologram_UIAction1");
	Action1UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	Action1UI->SetPosition({ -502.f, -86.f });
	Action1UI->SetSize({ 94.f, 94.f });
	Action1UI->SetVisible(false);
	// �ൿ�Է¿��� �ൿ 4���� �Է¹޴� UI(Ȧ�α׷� �г� �ڽ�)
	auto Action2UI = m_UIManager->CreateUI<UIPanel>(L"Hologram_UIAction2");
	Action2UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	Action2UI->SetPosition({ -323.f, -86.f });
	Action2UI->SetSize({ 94.f, 94.f });
	Action2UI->SetVisible(false);
	// �ൿ�Է¿��� �ൿ 4���� �Է¹޴� UI(Ȧ�α׷� �г� �ڽ�)
	auto Action3UI = m_UIManager->CreateUI<UIPanel>(L"Hologram_UIAction3");
	Action3UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	Action3UI->SetPosition({ -144.f, -86.f });
	Action3UI->SetSize({ 94.f, 94.f });
	Action3UI->SetVisible(false);

	HologramPanelUI->AddChildren(TimerUI, InputRemainedTurnUI, Action0UI, Action1UI, Action2UI, Action3UI);

	//// ��ǲ �г� (w)
	//auto InputPanel_w = m_UIManager->CreateUI<UIPanel>(L"InputPanel_w");
	//InputPanel_w->SetTexturePath(L"../Resources/Textures/Input_panel_w.png");
	//InputPanel_w->SetPosition({-295.f, -433.f});
	//InputPanel_w->SetSize({ 40.f, 36.f });
	//InputPanel_w->SetVisible(true);
	//// ��ǲ �г� (a)
	//auto InputPanel_a = m_UIManager->CreateUI<UIPanel>(L"InputPanel_a");
	//InputPanel_a->SetTexturePath(L"../Resources/Textures/Input_panel_a.png");
	//InputPanel_a->SetPosition({-333.5f, -467.f});
	//InputPanel_a->SetSize({ 39.f, 36.f });
	//InputPanel_a->SetVisible(true);
	//// ��ǲ �г� (s)
	//auto InputPanel_s = m_UIManager->CreateUI<UIPanel>(L"InputPanel_s");
	//InputPanel_s->SetTexturePath(L"../Resources/Textures/Input_panel_s.png");
	//InputPanel_s->SetPosition({-296.5f, -468.f});
	//InputPanel_s->SetSize({ 37.f, 37.f });
	//InputPanel_s->SetVisible(true);
	//// ��ǲ �г� (d)
	//auto InputPanel_d = m_UIManager->CreateUI<UIPanel>(L"InputPanel_d");
	//InputPanel_d->SetTexturePath(L"../Resources/Textures/Input_panel_d.png");
	//InputPanel_d->SetPosition({-260.5f, -468.f});
	//InputPanel_d->SetSize({ 37.f, 37.f });
	//InputPanel_d->SetVisible(true);
	//// ��ǲ �г� (q)
	//auto InputPanel_q = m_UIManager->CreateUI<UIPanel>(L"InputPanel_q");
	//InputPanel_q->SetTexturePath(L"../Resources/Textures/Input_panel_q.png");
	//InputPanel_q->SetPosition({-89.f, -456.f});
	//InputPanel_q->SetSize({ 40.f, 36.f });
	//InputPanel_q->SetVisible(true);
	//// ��ǲ �г� (e)
	//auto InputPanel_e = m_UIManager->CreateUI<UIPanel>(L"InputPanel_e");
	//InputPanel_e->SetTexturePath(L"../Resources/Textures/Input_panel_e.png");
	//InputPanel_e->SetPosition({111.f, -455.f});
	//InputPanel_e->SetSize({ 40.f, 40.f });
	//InputPanel_e->SetVisible(true);
	//// ��ǲ �г� (x)
	//auto InputPanel_x = m_UIManager->CreateUI<UIPanel>(L"InputPanel_x");
	//InputPanel_x->SetTexturePath(L"../Resources/Textures/Input_panel_x.png");
	//InputPanel_x->SetPosition({313.f, -455.f});
	//InputPanel_x->SetSize({ 42.f, 36.f });
	//InputPanel_x->SetVisible(true);

	// �ΰ��� �г�
	auto StagePanelUI = m_UIManager->CreateUI<UIPanel>(L"Turn_StagePanelUI");
	StagePanelUI->SetTexturePath(L"../Resources/Textures/Turn_bg.png");
	StagePanelUI->SetSize({ 1920.f, 1080.f });
	StagePanelUI->SetPosition({ 0.f,  0.f });
	StagePanelUI->SetScale({ 1.f, 1.f });

	// ���� ���� �ؽ�Ʈ UI (�ΰ��� �г� �ڽ�)
	auto PartsNumUI = m_UIManager->CreateUI<UIText>(L"Turn_PartsNumUI");
	PartsNumUI->SetPosition({ -800.f, 468.f });
	PartsNumUI->SetFont(L"KoPubWold����ü Bold");
	PartsNumUI->SetFontSize(50.f);
	PartsNumUI->SetColor(0x56b0e1);;
	PartsNumUI->SetText(std::to_wstring(0));

	// ���� �ϼ� UI(�ΰ��� �г� �ڽ�)
	auto RemainedTurnUI = m_UIManager->CreateUI<UIText>(L"Turn_RemainedTurnUI");
	RemainedTurnUI->SetPosition({ 0.f, 455.f });
	RemainedTurnUI->SetFont(L"KoPubWold����ü Bold");
	RemainedTurnUI->SetFontSize(50.f);
	RemainedTurnUI->SetColor(0x56b0e1);
	RemainedTurnUI->SetText(std::to_wstring(m_numTurn - 1));

	// host �ൿ 4����(�ΰ��� �г� �ڽ�)
	auto HostAction0UI = m_UIManager->CreateUI<UIPanel>(L"Turn_HostAction0");
	HostAction0UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	HostAction0UI->SetPosition({ -854.f, -451.f });
	HostAction0UI->SetSize({ 94.f, 94.f });
	// host �ൿ 4����(�ΰ��� �г� �ڽ�)
	auto HostAction1UI = m_UIManager->CreateUI<UIPanel>(L"Turn_HostAction1");
	HostAction1UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	HostAction1UI->SetPosition({ -709.f, -451.f });
	HostAction1UI->SetSize({ 94.f, 94.f });
	// host �ൿ 4����(�ΰ��� �г� �ڽ�)
	auto HostAction2UI = m_UIManager->CreateUI<UIPanel>(L"Turn_HostAction2");
	HostAction2UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	HostAction2UI->SetPosition({ -564.f, -451.f });
	HostAction2UI->SetSize({ 94.f, 94.f });
	// host �ൿ 4����(�ΰ��� �г� �ڽ�)
	auto HostAction3UI = m_UIManager->CreateUI<UIPanel>(L"Turn_HostAction3");
	HostAction3UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	HostAction3UI->SetPosition({ -419.f, -451.f });
	HostAction3UI->SetSize({ 94.f, 94.f });

	// guest �ൿ 4����(�ΰ��� �г� �ڽ�)
	auto guestAction0UI = m_UIManager->CreateUI<UIPanel>(L"Turn_GuestAction0");
	guestAction0UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	guestAction0UI->SetPosition({ 419.f, -451.f });
	guestAction0UI->SetSize({ 94.f, 94.f });
	// guest �ൿ 4����(�ΰ��� �г� �ڽ�)
	auto guestAction1UI = m_UIManager->CreateUI<UIPanel>(L"Turn_GuestAction1");
	guestAction1UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	guestAction1UI->SetPosition({ 564.f, -451.f });
	guestAction1UI->SetSize({ 94.f, 94.f });
	// guest �ൿ 4����(�ΰ��� �г� �ڽ�)
	auto guestAction2UI = m_UIManager->CreateUI<UIPanel>(L"Turn_GuestAction2");
	guestAction2UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	guestAction2UI->SetPosition({ 709.f, -451.f });
	guestAction2UI->SetSize({ 94.f, 94.f });
	// guest �ൿ 4����(�ΰ��� �г� �ڽ�)
	auto guestAction3UI = m_UIManager->CreateUI<UIPanel>(L"Turn_GuestAction3");
	guestAction3UI->SetTexturePath(L"../Resources/Textures/transparent.png");
	guestAction3UI->SetPosition({ 854.f, -451.f });
	guestAction3UI->SetSize({ 94.f, 94.f });

	StagePanelUI->AddChildren(PartsNumUI, RemainedTurnUI, HostAction0UI, HostAction1UI, HostAction2UI, HostAction3UI, guestAction0UI, guestAction1UI, guestAction2UI, guestAction3UI);
	StagePanelUI->SetAllVisible(false);

	// ���� ��ư ������ �� �ε� ��������Ʈ �ִϸ��̼�
	auto LoadingPanelUI = m_UIManager->CreateUI<UIPanel>(L"LoadingPanelUI");
	LoadingPanelUI->SetTexturePath(L"../Resources/Textures/Over_Anim_Waiting for call.png");
	LoadingPanelUI->SetSize({ 684.f, 128.f });
	LoadingPanelUI->SetPosition({ 0.f,  92.f });
	LoadingPanelUI->SetScale({ 1.f, 1.f });
	LoadingPanelUI->SetVisible(false);

	// ��ư ������ �� ��ư ��������Ʈ �ִϸ��̼�
	auto InputAnimALLUI = m_UIManager->CreateUI<UIPanel>(L"Input_AnimALLUI");
	InputAnimALLUI->SetTexturePath(L"../Resources/Textures/Input_Anim_ALL.png");
	InputAnimALLUI->SetSize({ 838.f, 148.f });
	InputAnimALLUI->SetPosition({ 21.f,  -330.f });
	InputAnimALLUI->SetScale({ 1.f, 1.f });
	InputAnimALLUI->SetVisible(true);

	// ���� �˾� UI
	auto ReadyPopUpUI = m_UIManager->CreateUI<UIFunctorPopUp>(L"PopUpUI_Ready");
	ReadyPopUpUI->SetTexturePath(L"../Resources/Textures/Input_p_warning.png");
	ReadyPopUpUI->SetSize({ 949.f, 548.f });
	ReadyPopUpUI->SetPosition({ 0.f, 0.f });
	ReadyPopUpUI->SetScale({ 1.f, 1.f });

	// �ൿ�� 4���� �� �Էµ��� ���� ���� �� �ְ� �ϴ� Functor
	auto ReadyClickPopUpFunctor = std::make_shared <UIReadyClickPopUpFunctor>();
	ReadyClickPopUpFunctor->SetTargetUI(ReadyPopUpUI);
	ReadyButtonUI->SetMouseClickFunctor(ReadyClickPopUpFunctor);

	// ���� �˾����� yes ��ư(���� �˾� �ڽ�)
	auto ReadyClickFunctor = std::make_shared<UIReadyClick>();
	auto ReadyOkUI = m_UIManager->CreateUI<UIButton>(L"PopUpUIButton_ReadyOk");
	ReadyOkUI->SetTexturePath(L"../Resources/Textures/Input_p_btn_ready.png");
	ReadyOkUI->SetSize({ 214.f, 91.f });
	ReadyOkUI->SetPosition({ -152.f, -133.f });
	ReadyOkUI->SetScale({ 1.f, 1.f });
	ReadyOkUI->SetMouseClickFunctor(ReadyClickFunctor);
	// ���� �˾����� no ��ư(���� �˾� �ڽ�)
	auto ReadyNoUI = m_UIManager->CreateUI<UIButton, UINoClick>(L"PopUpUIButton_ReadyNo");
	ReadyNoUI->SetTexturePath(L"../Resources/Textures/Input_p_b_return.png");
	ReadyNoUI->SetSize({ 214.f, 91.f });
	ReadyNoUI->SetPosition({ 146.f, -133.f });
	ReadyNoUI->SetScale({ 1.f, 1.f });

	ReadyPopUpUI->AddChildren(ReadyOkUI, ReadyNoUI);
	ReadyPopUpUI->SetVisible(false);

	// �̵��Ѵٰ� �˸��� �˾� UI + �ð� �ʰ��� �˸�
	auto ActionStartPopUpUI = m_UIManager->CreateUI<UIFunctorPopUp>(L"PopUpUI_ActionStart");
	ActionStartPopUpUI->SetTexturePath(L"../Resources/Textures/Input_p_action start.png");
	ActionStartPopUpUI->SetSize({ 949.f, 548.f });
	ActionStartPopUpUI->SetPosition({ 0.f, 0.f });
	ActionStartPopUpUI->SetScale({ 1.f, 1.f });
	ActionStartPopUpUI->SetVisible(false);

	// �������� Ŭ���� �˾� UI
	auto StageClearPopUpUI = m_UIManager->CreateUI<UIFunctorPopUp>(L"PopUpUI_StageClear");
	StageClearPopUpUI->SetTexturePath(L"../Resources/Textures/Turn_p_clear.png");
	StageClearPopUpUI->SetSize({ 1340.f, 769.f });
	StageClearPopUpUI->SetPosition({ 0.f, 0.f });
	StageClearPopUpUI->SetScale({ 1.f, 1.f });
	StageClearPopUpUI->SetVisible(false);

	// Ŭ���� �˾� ����Ʈ
	auto StageClearPopUpUI_B = m_UIManager->CreateUI<UIPanel>(L"StageClear_Rocket");  // rocket  21
	StageClearPopUpUI_B->SetTexturePath(L"../Resources/Textures/transparent.png");
	StageClearPopUpUI_B->SetSize({ 784.f, 421.f });
	StageClearPopUpUI_B->SetPosition({ 243.f, -18.f });
	StageClearPopUpUI_B->SetScale({ 1.f, 1.f });
	StageClearPopUpUI_B->SetVisible(false);

	// �ҿ� �� ���� Text
	auto UseTurnValueUI = m_UIManager->CreateUI<UIText>(L"StageClear_UseTurnValueUI");
	UseTurnValueUI->SetPosition({ -350.f, 84.f });
	UseTurnValueUI->SetFont(L"Arial");
	UseTurnValueUI->SetFontSize(36.f);
	UseTurnValueUI->SetColor(D2D1::ColorF::Black);
	std::wstring useTurnString = std::format(L"�ҿ� �� : {0} / �ִ� �� : {1}", m_useTurnValue, m_maxTurnValue);
	UseTurnValueUI->SetText(useTurnString);
	UseTurnValueUI->SetVisible(false);

	// ���� ��ǰ ���� Text
	auto GetPartValueUI = m_UIManager->CreateUI<UIText>(L"StageClear_GetPartValueUI");
	GetPartValueUI->SetPosition({ -342.f, -12.f });
	GetPartValueUI->SetFont(L"Arial");
	GetPartValueUI->SetFontSize(36.f);
	GetPartValueUI->SetColor(D2D1::ColorF::Black);
	std::wstring getPartString = std::format(L"Bear : {0} / Rabbit : {1}", hostPlayer->GetPart(), guestPlayer->GetPart());
	GetPartValueUI->SetText(getPartString);
	GetPartValueUI->SetVisible(false);

	// ���� ���� ���� �ۼ�Ʈ Text
	auto RocketProcessPercent = m_UIManager->CreateUI<UIText>(L"StageClear_RocketProcessPercent");
	RocketProcessPercent->SetPosition({ -357.f, -175.f });
	RocketProcessPercent->SetFont(L"Arial");
	RocketProcessPercent->SetFontSize(88.f);
	RocketProcessPercent->SetColor(D2D1::ColorF::Black);
	RocketProcessPercent->SetVisible(false);
	StageClearPopUpUI->AddChildren(StageClearPopUpUI_B, UseTurnValueUI, GetPartValueUI, RocketProcessPercent);

	// ���� �������� ���� �� ���� �˾� UI �ؽ�Ʈ
	auto NextStageUI = m_UIManager->CreateUI<UIButton>(L"NextStageUI");
	/*NextStageUI->SetTexturePath(L"../Resources/Textures/Input_p_btn_ready.png");
	NextStageUI->SetSize({ 214.f, 91.f });
	NextStageUI->SetPosition({ -152.f, -133.f });
	NextStageUI->SetScale({ 1.f, 1.f });
	StageClearPopUpUI->AddChildren(NextStageUI);*/

	StageClearPopUpUI->SetAllVisible(false);

	/// Init
	__super::Initialize();


	//
	m_dimUI->GetUIInstance().lock()->SetIsDim();
	m_dimUI->GetUIInstance().lock()->SetUseDim(true);

	// FadeInOut
	FadeInTestUI->GetUIInstance().lock()->SetIsFade();
	FadeInTestUI->GetUIInstance().lock()->SetFadeDurationTime(1.5f);
	m_tFadeInOut->GetUIInstance().lock()->FadeInStart();


	for (int i = 0; i < 4; i++)
	{
		CB_UIAnimationKeyframe keyframe;
		keyframe.m_Offset.x = 684.f * i;
		keyframe.m_Offset.y = 0.f;
		keyframe.m_TextureSize.x = 2736.f;
		keyframe.m_TextureSize.y = 128.f;
		keyframe.m_Size.x = 684.f;
		keyframe.m_Size.y = 128.f;
		keyframe.m_AnimationTime = 0.5f;
		LoadingPanelUI->AddKeyFrame(keyframe);
	}
	LoadingPanelUI->GetUIInstance().lock()->SetIsLoop(true);

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			CB_UIAnimationKeyframe keyframe;
			keyframe.m_Offset.x = 838.f * j;
			keyframe.m_Offset.y = 148.f * i;
			keyframe.m_TextureSize.x = 3352.f;
			keyframe.m_TextureSize.y = 1332.f;
			keyframe.m_Size.x = 838.f;
			keyframe.m_Size.y = 148.f;
			keyframe.m_AnimationTime = 0.15f;
			InputAnimALLUI->AddKeyFrame(keyframe);
		}
	}
	InputAnimALLUI->GetUIInstance().lock()->SetIsLoop(true);

	// Functor���� Player ��Ī�����ֱ�(action�� index�� �˱� ����)
	if (WorldManager::GetInstance()->IsHostServer())
	{
		ReadyClickFunctor->SetPlayer(hostPlayer);
		ReadyClickPopUpFunctor->SetPlayer(hostPlayer);
	}
	else
	{
		ReadyClickFunctor->SetPlayer(guestPlayer);
		ReadyClickPopUpFunctor->SetPlayer(guestPlayer);
	}

	// Fade In Out
	GetFadeInOut()->GetUIInstance().lock()->FadeInStart();


	m_hostFire->PlayParticle(false);
	//m_hostFire->SetPosition({ 0.f, -200.f, 0.f });
	m_guestFire->PlayParticle(false);
	//m_guestFire->SetPosition({ 0.f, -200.f, 0.f });
}

void PuzzleWorld::Update(float deltaTime)
{
	__super::Update(deltaTime);

	std::shared_ptr<PlayerObject> hostPlayer = m_hostPlayer.lock();
	std::shared_ptr<PlayerObject> guestPlayer = m_guestPlayer.lock();

	m_divisionMap->Update();

	// Fade in out
	if (m_tFadeInOut != nullptr)
	{
		m_tFadeInOut->GetUIInstance().lock()->UpdateFadeInOut(deltaTime);
	}

	if (m_bSendStartTurn)
	{
		static float fixedDeltaTime = 0.f;
		fixedDeltaTime += deltaTime;

		if (fixedDeltaTime >= 1.f)
		{
			fixedDeltaTime = 0.f;
			m_bSendStartTurn = false;
			WorldManager::GetInstance()->PushSendQueue(
				WorldManager::GetInstance()->SerializeBuffer(sizeof(PacketC2S_StartTurn), C2S_START_TURN, nullptr),
				sizeof(PacketC2S_StartTurn));
		}
	}

	// ȣ��Ʈ �÷��̾� �Ͽ��� �� ü���� ��� ������ ��
	if (hostPlayer->GetTurn() && hostPlayer->GetWaitingChange())
	{
		if (!guestPlayer->GetComponentByTypeName<GridMovementComponent>().lock()->GetIsMove() &&
			!guestPlayer->GetComponentByTypeName<GridMovementComponent>().lock()->GetIsCrashing())
		{
			hostPlayer->Network_ChangeTurn();
			hostPlayer->SetWaitingChange(false);
			m_bSendStartTurn = true;
		}
	}
	// �Խ�Ʈ �÷��̾� �Ͽ��� �� ü���� ��� ������ ��
	if (guestPlayer->GetTurn() && guestPlayer->GetWaitingChange())
	{
		if (!hostPlayer->GetComponentByTypeName<GridMovementComponent>().lock()->GetIsMove() &&
			!hostPlayer->GetComponentByTypeName<GridMovementComponent>().lock()->GetIsCrashing())
		{
			guestPlayer->Network_ChangeTurn();
			guestPlayer->SetWaitingChange(false);
			m_bSendStartTurn = true;
		}
	}

	// todo ä�� UI ���� ���Ƴ����ִ°�(�ٵ� Update���� ���Ƽ� ������
	if (WorldManager::GetInstance()->IsHostServer())
	{
		if (hostPlayer->GetLoadingIndex() == 4)
		{
			auto ui = m_UIManager->GetUIObject<UIButton>(L"InputAction_ReadyButtonUI");
			ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_Anim_ReadyBtn.png");

			// Ȱ��ȭ ��ư �ִϸ��̼�
			if (ui.lock()->GetUIInstance().lock()->GetKeyFrameSize() <= 0)
			{
				for (int i = 0; i < 3; i++)
				{
					CB_UIAnimationKeyframe keyframe;
					keyframe.m_Offset.x = 228.f * i;
					keyframe.m_Offset.y = 0.f;
					keyframe.m_TextureSize.x = 912.f;
					keyframe.m_TextureSize.y = 206.f;
					keyframe.m_Size.x = 228.f;
					keyframe.m_Size.y = 206.f;
					keyframe.m_AnimationTime = 0.04f;
					ui.lock()->AddKeyFrame(keyframe);
				}

				CB_UIAnimationKeyframe keyframe;
				keyframe.m_Offset.x = 228.f * 3;
				keyframe.m_Offset.y = 0.f;
				keyframe.m_TextureSize.x = 912.f;
				keyframe.m_TextureSize.y = 206.f;
				keyframe.m_Size.x = 228.f;
				keyframe.m_Size.y = 206.f;
				keyframe.m_AnimationTime = 0.2f;
				ui.lock()->AddKeyFrame(keyframe);

				ui.lock()->GetUIInstance().lock()->SetIsLoop(false);
			}
		}
		else if (hostPlayer->GetLoadingIndex() < 4)
		{
			auto ui = m_UIManager->GetUIObject<UIButton>(L"InputAction_ReadyButtonUI");
			ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_ReadyBtn inactive.png");
			ui.lock()->GetUIInstance().lock()->ClearKeyFrame();
		}
	}

	else
	{
		if (guestPlayer->GetLoadingIndex() == 4)
		{
			auto ui = m_UIManager->GetUIObject<UIButton>(L"InputAction_ReadyButtonUI");
			ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_Anim_ReadyBtn.png");

			// Ȱ��ȭ ��ư �ִϸ��̼�
			if (ui.lock()->GetUIInstance().lock()->GetKeyFrameSize() <= 0)
			{
				for (int i = 0; i < 3; i++)
				{
					CB_UIAnimationKeyframe keyframe;
					keyframe.m_Offset.x = 228.f * i;
					keyframe.m_Offset.y = 0.f;
					keyframe.m_TextureSize.x = 912.f;
					keyframe.m_TextureSize.y = 206.f;
					keyframe.m_Size.x = 228.f;
					keyframe.m_Size.y = 206.f;
					keyframe.m_AnimationTime = 0.04f;
					ui.lock()->AddKeyFrame(keyframe);
				}

				CB_UIAnimationKeyframe keyframe;
				keyframe.m_Offset.x = 228.f * 3;
				keyframe.m_Offset.y = 0.f;
				keyframe.m_TextureSize.x = 912.f;
				keyframe.m_TextureSize.y = 206.f;
				keyframe.m_Size.x = 228.f;
				keyframe.m_Size.y = 206.f;
				keyframe.m_AnimationTime = 0.2f;
				ui.lock()->AddKeyFrame(keyframe);

				ui.lock()->GetUIInstance().lock()->SetIsLoop(false);
			}
		}
		else if (guestPlayer->GetLoadingIndex() < 4)
		{
			auto ui = m_UIManager->GetUIObject<UIButton>(L"InputAction_ReadyButtonUI");
			ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_ReadyBtn inactive.png");
			ui.lock()->GetUIInstance().lock()->ClearKeyFrame();
		}
	}

	// Ÿ�̸� ����
	if (m_bTimerRun)
	{
		m_timeOut -= deltaTime;

		std::wstringstream wss;
		wss << std::fixed << std::setprecision(2) << m_timeOut;
		std::wstring result = wss.str();
		auto ui = m_UIManager->GetUIObject<UIText>(L"Hologram_TimerUI");
		ui.lock()->SetText(result);

		// ���� �ð� ������ ��
		if (m_timeOut <= 0.f)
		{
			SoundManager::GetInstance()->PlaySound("../Resources/Sound/Effect/Timer_Over.mp3");

			ui.lock()->SetAllVisible(false);

			m_bTimerRun = false;
			if (WorldManager::GetInstance()->IsHostServer())
			{
				hostPlayer->TimeOut();
			}
			else
			{
				guestPlayer->TimeOut();
			}

			WorldManager::GetInstance()->PushSendQueue(
				WorldManager::GetInstance()->SerializeBuffer(sizeof(PacketC2S_READY), C2S_READY, nullptr),
				sizeof(PacketC2S_READY));
		}
	}

	// ���� ������ 3�ʰ� �˾� �����ֱ�
	if (m_bShowGameStart)
	{
		auto loadingPanelUI = m_UIManager->GetUIObject<UIPanel>(L"LoadingPanelUI");
		loadingPanelUI.lock()->SetAllVisible(false);

		auto actionStartPopUpUI = m_UIManager->GetUIObject<UIFunctorPopUp>(L"PopUpUI_ActionStart");
		auto readyPopUpUI = m_UIManager->GetUIObject<UIFunctorPopUp>(L"PopUpUI_Ready");
		readyPopUpUI.lock()->SetAllVisible(false);


		if ((WorldManager::GetInstance()->IsHostServer() && !hostPlayer->GetReadyState())
			|| (!WorldManager::GetInstance()->IsHostServer() && !guestPlayer->GetReadyState()))
		{
			actionStartPopUpUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_p_time limit exceeded.png");
		}
		else if ((WorldManager::GetInstance()->IsHostServer() && hostPlayer->GetReadyState())
			|| (!WorldManager::GetInstance()->IsHostServer() && guestPlayer->GetReadyState()))
		{
			actionStartPopUpUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_p_action start.png");
		}
		actionStartPopUpUI.lock()->SetAllVisible(true);

		static float fixed = 0.0f;

		fixed += deltaTime;

		if (fixed >= 1.0f)
		{
			m_showGameStart -= 1.0f;
			fixed -= 1.0f;
		}

		if (m_showGameStart <= 0.f)
		{
			// todo ä�� ���⼭ ���� �����ؾ��ϱ� ������ ���� ���۽� �ʱ�ȭ�� ���� ������
			actionStartPopUpUI.lock()->SetAllVisible(false);

			hostPlayer->SetReadyState(false);
			guestPlayer->SetReadyState(false);
			hostPlayer->SetExecuteBehavior(false);
			guestPlayer->SetExecuteBehavior(false);

			m_bGameRun = true;
			m_bShowGameStart = false;

			// UI �ٲ�ġ��
			UIChange();

			fixed = 0.0f;

			m_showGameStart = SHOW_GAME_START;
			m_timeOut = TIMEOUT;

			WorldManager::GetInstance()->PushSendQueue(
				WorldManager::GetInstance()->SerializeBuffer(sizeof(PacketC2S_StartTurn), C2S_START_TURN, nullptr),
				sizeof(PacketC2S_StartTurn));
		}
	}
}

// ��ǥ ������ �����ߴ��� �� �ߴ���
bool PuzzleWorld::GetGoalIn()
{
	GridPos hostGrid = m_gridManager->ConvertToGridPos(m_hostPlayer.lock()->GetWorldPosition());
	GridPos guestGrid = m_gridManager->ConvertToGridPos(m_guestPlayer.lock()->GetWorldPosition());


	if ((hostGrid.x == m_goalGridPos[0].x && hostGrid.z == m_goalGridPos[0].z &&
		guestGrid.x == m_goalGridPos[1].x && guestGrid.z == m_goalGridPos[1].z)
		|| (hostGrid.x == m_goalGridPos[1].x && hostGrid.z == m_goalGridPos[1].z &&
			guestGrid.x == m_goalGridPos[0].x && guestGrid.z == m_goalGridPos[0].z))
		return true;
	return false;

}

std::shared_ptr<PlayerObject> PuzzleWorld::GetOtherPlayer(PlayerObject* player)
{
	assert(m_hostPlayer.lock());
	assert(m_guestPlayer.lock());

	if (player == m_hostPlayer.lock().get())
		return m_guestPlayer.lock();
	else
		return m_hostPlayer.lock();
}

void PuzzleWorld::OnEnter()
{
	InputManager::GetInstance()->AddInputProcesser(this);

	World::OnEnter();
	InputManager::GetInstance()->GetMouse()->SetMode(Mouse::MODE_ABSOLUTE);

	auto pStage1 = std::dynamic_pointer_cast<Stage1World>(WorldManager::GetInstance()->GetCurWorld());
	if (pStage1)
		return;
	WorldManager::GetInstance()->PushSendQueue(
		WorldManager::GetInstance()->SerializeBuffer(sizeof(PacketC2S_IsAllEnter), C2S_IS_ALL_ENTER, nullptr),
		sizeof(PacketC2S_IsAllEnter));
}

void PuzzleWorld::OnExit()
{
	InputManager::GetInstance()->RemoveInputProcesser(this);

	World::OnExit();

}

void PuzzleWorld::HandleEvent(Event* event)
{
	switch (event->eventID)
	{
	case eEventType::DELETE_OBJECT:
		DeleteGameObject(event->GetData<const char*>());
		break;
	case eEventType::CHANGE_WORLD:
		WorldManager::GetInstance()->ChangeWorld(event->GetData<eWorldType>());
		break;
	}
}

void PuzzleWorld::OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker,
	const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker)
{
	// ġƮŰ
	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F1))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STAGE1);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F2))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STAGE2);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F3))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STAGE3);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F4))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STAGE4);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F5))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STAGE5);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F6))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STARTCUTSCECNE);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F7))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::STARTFREEWORLD);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F8))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::ENDCUTSCENE);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F9))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::ENDFREEWORLD);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F10))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::CREDIT);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F11))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::GAMEOVERWORLD);
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::F11))
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::MAIN);
	}

	// �Է� ���� �� ����ó��
	// ����ó�� 1 : �ΰ��� �������� ��
	if (m_bGameRun)
	{
		m_bBanActionInput = true;
		return;
	}

	// ����ó�� 2 : ���� �����Ѵٰ� �˾� ���� ��
	if (m_bShowGameStart)
	{
		m_bBanActionInput = true;
		return;
	}
	// ����ó�� 2 - 1 : ��������1 ���̵� �˾� ���� ��
	if (m_bStage1PopUp)
	{
		// todo ä�� : ����
		m_bBanActionInput = true;
		return;
	}
	else if (!m_bStage1PopUp)
	{
		m_bBanActionInput = false;
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Keys::Tab))
	{

		if (!m_bGameRun)
		{
			// Ȧ�α׷� ����
			auto ui = m_UIManager->GetUIObject<UIPanel>(L"Hologram_PanelUI");
			if (ui.lock()->GetVisible())
			{
				ui.lock()->SetAllVisible(false);

				// Dim
				m_dimUI->GetUIInstance().lock()->SetUseDim(false);
			}
			else if (!ui.lock()->GetVisible())
			{
				ui.lock()->SetAllVisible(true);

				// Dim
				m_dimUI->GetUIInstance().lock()->SetUseDim(true);
			}
		}
	}

	// ����ó�� 3 : �÷��̾ ������� �� ��
	if ((WorldManager::GetInstance()->IsHostServer() && m_hostPlayer.lock()->GetReadyState())
		|| (!WorldManager::GetInstance()->IsHostServer() && m_guestPlayer.lock()->GetReadyState()))
	{
		m_bBanActionInput = true;
		return;
	}

	// ����ó�� 4 : �����ư ������ �˾� ������ ���� ���� ���ϰ� ����ó��
	auto readyPopUpUI = m_UIManager->GetUIObject<UIFunctorPopUp>(L"PopUpUI_Ready");
	if (readyPopUpUI.lock()->GetVisible())
	{
		m_bBanActionInput = true;
		return;
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::X))
	{
		std::string str = "Hologram_UIAction";
		if (WorldManager::GetInstance()->IsHostServer())
		{
			if (m_hostPlayer.lock()->GetLoadingIndex() - 1 < 0)
				return;
			str += std::to_string(m_hostPlayer.lock()->GetLoadingIndex() - 1);
		}
		else
		{
			if (m_guestPlayer.lock()->GetLoadingIndex() - 1 < 0)
				return;
			str += std::to_string(m_guestPlayer.lock()->GetLoadingIndex() - 1);
		}

		auto ui = m_UIManager->GetUIObject<UIPanel>(std::wstring().assign(str.begin(), str.end()));
		ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/transparent.png");
		m_bBanActionInput = false;
	}

	// ����ó�� 5 : �̹� �ൿ 4���� �Է� �Ǿ����� ��(UI�ε��� ����� ����ó�� ����)
	if ((WorldManager::GetInstance()->IsHostServer() && m_hostPlayer.lock()->GetLoadingIndex() == 4)
		|| (!WorldManager::GetInstance()->IsHostServer() && m_guestPlayer.lock()->GetLoadingIndex() == 4))
		return;

	int loadingIndex;
	if (WorldManager::GetInstance()->IsHostServer())
	{
		loadingIndex = m_hostPlayer.lock()->GetLoadingIndex();
	}
	else
	{
		loadingIndex = m_guestPlayer.lock()->GetLoadingIndex();
	}

	if (KeyTracker.IsKeyPressed(DirectX::Keyboard::W))
	{
		auto ui = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Hologram_UIAction", std::to_string(loadingIndex)));
		ui.lock()->SetVisible(true);
		ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_action_w.png");
	}
	else if (KeyTracker.IsKeyPressed(DirectX::Keyboard::S))
	{
		auto ui = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Hologram_UIAction", std::to_string(loadingIndex)));
		ui.lock()->SetVisible(true);
		ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_action_s.png");
	}
	else if (KeyTracker.IsKeyPressed(DirectX::Keyboard::D))
	{
		auto ui = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Hologram_UIAction", std::to_string(loadingIndex)));
		ui.lock()->SetVisible(true);
		ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_action_d.png");
	}
	else if (KeyTracker.IsKeyPressed(DirectX::Keyboard::A))
	{
		auto ui = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Hologram_UIAction", std::to_string(loadingIndex)));
		ui.lock()->SetVisible(true);
		ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_action_a.png");
	}
	else if (KeyTracker.IsKeyPressed(DirectX::Keyboard::Q))
	{
		auto ui = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Hologram_UIAction", std::to_string(loadingIndex)));
		ui.lock()->SetVisible(true);
		ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_action_q(wait).png");
	}
	else if (KeyTracker.IsKeyPressed(DirectX::Keyboard::E))
	{
		auto ui = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Hologram_UIAction", std::to_string(loadingIndex)));
		ui.lock()->SetVisible(true);
		ui.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_action_e(rise).png");
	}
}

// �Ѵ� �����ư�� ������ �� Ȧ�α׷� UI ���߰� ���� �����ϰ� �ϱ�
void PuzzleWorld::Network_ClientsAllReady(char* pData)
{
	m_bShowGameStart = true;
	m_bTimerRun = false;

	//InputManager::GetInstance()->GetMouse()->SetMode(Mouse::MODE_RELATIVE);

	Network_SendAction();

	m_hostPlayer.lock()->ResetIndex();
	m_guestPlayer.lock()->ResetIndex();

	delete[] pData;
}

// ��� Stage�� �������� �� Ÿ�̸� ����
void PuzzleWorld::Network_ClientsAllEnter(char* pData)
{
	InputManager::GetInstance()->GetMouse()->SetMode(Mouse::MODE_ABSOLUTE);

	m_bTimerRun = true;
	m_timeOut = TIMEOUT;
	m_showGameStart = SHOW_GAME_START;
	m_bGameRun = false;
	m_bShowGameStart = false;
	m_bShowStageClear = false;
	m_bBanActionInput = false;

	delete[] pData;
}

// �������� �ൿ�� �ִ� �Լ�.
void PuzzleWorld::ProceedAction(char* pData)
{
	InputManager::GetInstance()->GetMouse()->SetMode(Mouse::MODE_RELATIVE);

	if (m_hostPlayer.lock()->GetTurn())
	{
		m_hostPlayer.lock()->ProceedAction();
	}
	else if (m_guestPlayer.lock()->GetTurn())
	{
		m_guestPlayer.lock()->ProceedAction();
	}

	delete pData;
}

bool PuzzleWorld::CheckRemainingAction()
{
	// �ൿ �����ִ��� Ȯ��
	if (m_hostPlayer.lock()->CheckActionEnd() && m_guestPlayer.lock()->CheckActionEnd())
	{
		m_useTurnValue++;
		m_numTurn--;
		InitMiniMap();
		// todo ä�� :��ǥ������ �����߰�, ������ 0���� ������  && m_numParts == 0
		if (GetGoalIn() && m_numParts == 0)
		{
			CheckGoalIn(true);
			return false;
		}

		if (m_numTurn == 0)
		{
			CheckGoalIn(false);
			return false;
		}
		NetWork_ActionEnd();
		return false;
	}
	return true;
}

bool PuzzleWorld::CheckGoalIn(bool isGoalIn)
{
	InputManager::GetInstance()->GetMouse()->SetMode(Mouse::MODE_ABSOLUTE);

	if (isGoalIn)
	{
		m_bShowStageClear = true;
		m_bGameRun = false;
		m_bBanActionInput = false;
		m_bTimerRun = false;
		m_timeOut = TIMEOUT;
		m_showGameStart = SHOW_GAME_START;

		m_hostPlayer.lock()->ResetIndex();
		m_guestPlayer.lock()->ResetIndex();
	}
	else
	{
		EventManager::GetInstance()->SendEvent(eEventType::CHANGE_WORLD, this, eWorldType::GAMEOVERWORLD);
	}
	return true;
}

void PuzzleWorld::SubNumParts()
{
	m_numParts--;
	auto partsNumUI = m_UIManager->GetUIObject<UIText>(L"Turn_PartsNumUI");
	m_gainedParts++;
	partsNumUI.lock()->SetText(std::to_wstring(m_gainedParts));

	if (m_hostPlayer.lock()->GetTurn())
		m_hostPlayer.lock()->AddPart();
	else if (m_guestPlayer.lock()->GetTurn())
		m_guestPlayer.lock()->AddPart();
}

void PuzzleWorld::InitMiniMap()
{
	auto miniMapUI = m_UIManager->GetUIObject<UIPanel>(L"Hologram_MiniMapUI");
	int x = miniMapUI.lock()->GetSize().x / m_gridManager->GetNumCol();
	int z = miniMapUI.lock()->GetSize().y / m_gridManager->GetNumRow();

	auto miniMapPlayerUI = m_UIManager->GetUIObject<UIPanel>(L"MiniMap_PlayerUI");
	auto miniMapFriendUI = m_UIManager->GetUIObject<UIPanel>(L"MiniMap_FriendUI");

	int partsNum = 0;

	std::vector<std::pair<MinimapPos, eMinimapType>>& minimapInfo = m_gridManager->GetMinimapInfo();
	for (short i = 0; i < minimapInfo.size(); i++)
	{
		// position
		if (minimapInfo[i].second == eMinimapType::HOST)
		{
			float xPos = (float)x * (float)minimapInfo[i].first.x + (x / 2);
			float zPos = (float)z * (float)minimapInfo[i].first.z + (z / 2);
			if (WorldManager::GetInstance()->IsHostServer())
				miniMapPlayerUI.lock()->SetPosition({ xPos - miniMapUI.lock()->GetSize().x / 2, zPos - miniMapUI.lock()->GetSize().y / 2 });
			else
				miniMapFriendUI.lock()->SetPosition({ xPos - miniMapUI.lock()->GetSize().x / 2, zPos - miniMapUI.lock()->GetSize().y / 2 });
		}
		else if (minimapInfo[i].second == eMinimapType::GUEST)
		{
			float xPos = (float)x * (float)minimapInfo[i].first.x + (x / 2);
			float zPos = (float)z * (float)minimapInfo[i].first.z + (z / 2);

			if (WorldManager::GetInstance()->IsHostServer())
				miniMapFriendUI.lock()->SetPosition({ xPos - miniMapUI.lock()->GetSize().x / 2, zPos - miniMapUI.lock()->GetSize().y / 2 });
			else
				miniMapPlayerUI.lock()->SetPosition({ xPos - miniMapUI.lock()->GetSize().x / 2, zPos - miniMapUI.lock()->GetSize().y / 2 });
		}
		else if (minimapInfo[i].second == eMinimapType::OVERLAP)
		{
			float xPos = (float)x * (float)minimapInfo[i].first.x + (x / 2);
			float zPos = (float)z * (float)minimapInfo[i].first.z + (z / 2);

			miniMapFriendUI.lock()->SetPosition({ xPos - miniMapUI.lock()->GetSize().x / 2, zPos - miniMapUI.lock()->GetSize().y / 2 });
			miniMapPlayerUI.lock()->SetPosition({ xPos - miniMapUI.lock()->GetSize().x / 2, zPos - miniMapUI.lock()->GetSize().y / 2 });
		}
		else if (minimapInfo[i].second == eMinimapType::PART)
		{
			auto targetUI = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("MiniMap_TargetUI", std::to_string(partsNum++)));
			targetUI.lock()->SetVisible(true);
			float xPos = (float)x * (float)minimapInfo[i].first.x + (x / 2);
			float zPos = (float)z * (float)minimapInfo[i].first.z + (z / 2);
			targetUI.lock()->SetPosition({ xPos - miniMapUI.lock()->GetSize().x / 2, zPos - miniMapUI.lock()->GetSize().y / 2 });
			targetUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_target token.png");
		}
	}

	while (m_stageParts != partsNum)
	{
		auto targetUI = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("MiniMap_TargetUI", std::to_string(partsNum++)));
		targetUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/transparent.png");
		targetUI.lock()->SetPosition({ 0.f, 0.f });
	}

	m_gridManager->ClearMinimapInfo();
}

void PuzzleWorld::InitAll()
{
	m_bGameRun = false;
	m_bTimerRun = false;
	m_bShowGameStart = false;
	m_bShowStageClear = false; // false; �׽�Ʈ�� ���� true .. �ٽ� false�� �ٲ����
	m_bBanActionInput = false;
	m_bStage1PopUp = false;
	m_timeOut = TIMEOUT;
	m_showGameStart = SHOW_GAME_START;
	m_showStageClear = 0.f;
	m_gainedParts = 0;

	m_playerPositions = { {0,0,0},{0,0,0} };
	m_numParts = 0;
	m_numTurn = 0;

	if (m_hostPlayer.lock())
		m_hostPlayer.lock()->InitPlayer();
	if (m_guestPlayer.lock())
		m_guestPlayer.lock()->InitPlayer();
}

std::wstring PuzzleWorld::ConvertToActionUI(Vector3 vec, bool actionDone)
{
	if (!actionDone)
	{
		if (vec == grid::X_UP)
			return L"../Resources/Textures/Input_action_d.png";

		else if (vec == grid::X_DOWN)
			return L"../Resources/Textures/Input_action_a.png";

		else if (vec == grid::Y_UP)
			return L"../Resources/Textures/Input_action_e(rise).png";

		else if (vec == grid::Z_UP)
			return L"../Resources/Textures/Input_action_w.png";

		else if (vec == grid::Z_DOWN)
			return L"../Resources/Textures/Input_action_s.png";

		else if (vec == grid::STAY)
			return L"../Resources/Textures/Input_action_q(wait).png";
	}
	else if (actionDone)
	{
		if (vec == grid::X_UP)
			return L"../Resources/Textures/Turn_action_d.png";

		else if (vec == grid::X_DOWN)
			return L"../Resources/Textures/Turn_action_a.png";

		else if (vec == grid::Y_UP)
			return L"../Resources/Textures/Turn_action_e(rise).png";

		else if (vec == grid::Z_UP)
			return L"../Resources/Textures/Turn_action_w.png";

		else if (vec == grid::Z_DOWN)
			return L"../Resources/Textures/Turn_action_s.png";

		else if (vec == grid::STAY)
			return L"../Resources/Textures/Turn_Action_q(wait).png";
	}
}

void PuzzleWorld::UIChange()
{
	if (m_bGameRun)
	{
		// �ൿ�Է� �� UI
		auto enterActionUI = m_UIManager->GetUIObject<UIPanel>(L"InputAction_PanelUI");
		enterActionUI.lock()->SetAllVisible(false);

		auto hologramUI = m_UIManager->GetUIObject<UIPanel>(L"Hologram_PanelUI");
		hologramUI.lock()->SetAllVisible(false);

		// Dim
		m_dimUI->GetUIInstance().lock()->SetUseDim(false);

		auto loadingActionPanelUI = m_UIManager->GetUIObject<UIPanel>(L"InputAction_LoadingActionPanelUI");
		loadingActionPanelUI.lock()->SetAllVisible(false);

		// �ΰ��� UI
		auto StageUI = m_UIManager->GetUIObject<UIPanel>(L"Turn_StagePanelUI");
		StageUI.lock()->SetAllVisible(true);

		auto InputAnimALLUI = m_UIManager->GetUIObject<UIPanel>(L"Input_AnimALLUI");
		InputAnimALLUI.lock()->SetAllVisible(false);

		m_hostPlayer.lock()->SetExecuteBehavior(true);
		m_guestPlayer.lock()->SetExecuteBehavior(true);

	}
	else
	{
		// �ൿ�Է� �� UI
		auto enterActionUI = m_UIManager->GetUIObject<UIPanel>(L"InputAction_PanelUI");
		enterActionUI.lock()->SetAllVisible(true);

		auto hologramUI = m_UIManager->GetUIObject<UIPanel>(L"Hologram_PanelUI");
		hologramUI.lock()->SetAllVisible(true);

		// Dim
		m_dimUI->GetUIInstance().lock()->SetUseDim(true);

		auto loadingActionPanelUI = m_UIManager->GetUIObject<UIPanel>(L"InputAction_LoadingActionPanelUI");
		loadingActionPanelUI.lock()->SetAllVisible(false);

		// �ΰ��� UI
		auto StageUI = m_UIManager->GetUIObject<UIPanel>(L"Turn_StagePanelUI");
		StageUI.lock()->SetAllVisible(false);

		auto InputAnimALLUI = m_UIManager->GetUIObject<UIPanel>(L"Input_AnimALLUI");
		InputAnimALLUI.lock()->SetAllVisible(true);

		m_hostPlayer.lock()->SetExecuteBehavior(false);
		m_guestPlayer.lock()->SetExecuteBehavior(false);
	}
}

// �����κ��� ������ ������ �޾��� �� ���� �÷��̾���� ���� �������ش�.
void PuzzleWorld::Network_SetTurn(char* pData)
{
	PacketS2C_SetTurn* pSetTurn = reinterpret_cast<PacketS2C_SetTurn*>(pData);

	auto firstUI = m_UIManager->GetUIObject<UIPanel>(L"InputAction_FirstTurnActiveUI");

	if (pSetTurn->setTurn == '0')
	{
		m_hostPlayer.lock()->SetTurn(true);
		m_guestPlayer.lock()->SetTurn(false);
		// �� �ٲ�� ���� UI �ٲ��ֱ�
		if(WorldManager::GetInstance()->IsHostServer())
			firstUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_First Lamp active.png");
		else
			firstUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_First Lamp2 active.png");
	}
	else if (pSetTurn->setTurn == '1')
	{
		m_hostPlayer.lock()->SetTurn(false);
		m_guestPlayer.lock()->SetTurn(true);

		if (!WorldManager::GetInstance()->IsHostServer())
			firstUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_First Lamp active.png");
		else
			firstUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/Input_First Lamp2 active.png");
	}
	delete pData;
}

// �������� ���� 4���� �ൿ�� �� �÷��̾��� m_action�̶�� �����̳ʿ� �־��ش�.
void PuzzleWorld::Network_SetUIAction(char* pData)
{
	// ��ǥ? �޾ƿͼ� UI �ؽ��� ���Ƴ����ֱ�
	PacketS2C_BroadcastAction* pBroadcastAction = reinterpret_cast<PacketS2C_BroadcastAction*>(pData);

	if (pBroadcastAction->who == '0')	// host
	{
		m_hostPlayer.lock()->SetActions(pBroadcastAction->action);
		for (int i = 0; i < 4; i++)
		{
			auto ui = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Turn_HostAction", std::to_string(i)));
			ui.lock()->GetUIInstance().lock()->LoadTextureResource(ConvertToActionUI(m_hostPlayer.lock()->GetAction(i), false));
		}
	}
	else if (pBroadcastAction->who == '1')
	{
		m_guestPlayer.lock()->SetActions(pBroadcastAction->action);
		for (int i = 0; i < 4; i++)
		{
			auto ui = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Turn_GuestAction", std::to_string(i)));
			ui.lock()->GetUIInstance().lock()->LoadTextureResource(ConvertToActionUI(m_guestPlayer.lock()->GetAction(i), false));
		}
	}
	delete pData;
}

// �������� ������ �� PlayerObject changeTurn������
void PuzzleWorld::Network_MovementEnd(char* pData)
{
	if (m_hostPlayer.lock()->GetTurn())
	{
		m_hostPlayer.lock()->Network_ChangeTurn();
	}
	else if (m_guestPlayer.lock()->GetTurn())
	{
		m_guestPlayer.lock()->Network_ChangeTurn();
	}
	delete[] pData;
}

// ������ ���� �ൿ �����ֱ�
void PuzzleWorld::Network_SendAction()
{
	if (WorldManager::GetInstance()->IsHostServer())
	{
		m_hostPlayer.lock()->Network_SendAction();
	}
	else
	{
		m_guestPlayer.lock()->Network_SendAction();
	}
}

// Action�� ������ �� �ൿ�Է�â ������ �� ���� �Էµƴ��͵� �ʱ�ȭ
void PuzzleWorld::NetWork_ActionEnd()
{
	// �� ������ �� ���⼭ �ʱ�ȭ (UI, �ε���, Ÿ�̸�...)
	m_bGameRun = false;
	m_bBanActionInput = false;
	m_timeOut = TIMEOUT;
	m_showGameStart = SHOW_GAME_START;

	UIChange();

	m_hostPlayer.lock()->ResetIndex();
	m_guestPlayer.lock()->ResetIndex();

	m_hostPlayer.lock()->SetReadyState(false);
	m_guestPlayer.lock()->SetReadyState(false);

	for (int i = 0; i < 4; i++)
	{
		auto actionUI = m_UIManager->GetUIObject<UIPanel>(ExtractUIName("Hologram_UIAction", std::to_string(i)));
		actionUI.lock()->GetUIInstance().lock()->LoadTextureResource(L"../Resources/Textures/transparent.png");
		actionUI.lock()->SetAllVisible(false);
	}

	auto trunUI1 = m_UIManager->GetUIObject<UIText>(L"Hologram_RemainedTurnUI");
	trunUI1.lock()->SetText(std::to_wstring(m_numTurn - 1));

	auto trunUI2 = m_UIManager->GetUIObject<UIText>(L"Turn_RemainedTurnUI");
	trunUI2.lock()->SetText(std::to_wstring(m_numTurn - 1));

	auto getPartUI = m_UIManager->GetUIObject<UIText>(L"StageClear_GetPartValueUI");
	std::wstring getPartString = std::format(L"Bear : {0} / Rabbit : {1}", m_hostPlayer.lock()->GetPart(), m_guestPlayer.lock()->GetPart());
	getPartUI.lock()->SetText(getPartString);

	auto turnValueUI = m_UIManager->GetUIObject<UIText>(L"StageClear_UseTurnValueUI");
	std::wstring useTurnString = std::format(L"�ҿ� �� : {0} / �ִ� �� : {1}", m_useTurnValue, m_maxTurnValue);
	turnValueUI.lock()->SetText(useTurnString);

	WorldManager::GetInstance()->PushSendQueue(
		WorldManager::GetInstance()->SerializeBuffer(sizeof(PacketC2S_IsAllEnter), C2S_IS_ALL_ENTER, nullptr),
		sizeof(PacketC2S_IsAllEnter));
}

std::wstring PuzzleWorld::ExtractUIName(std::string uiName, std::string index)
{
	if (WorldManager::GetInstance()->IsHostServer())
		uiName += index;
	else uiName += index;

	return std::wstring().assign(uiName.begin(), uiName.end());
}
