#include "pch.h"
#include "CommonApp.h"

#include <imgui.h>
#include <thread>

#include "InputManager.h"
#include "WorldManager.h"
#include "SoundManager.h"
#include "PhysicsManager.h"
#include "EventManager.h"

#include "../D3DRenderer/D3DRenderManager.h"
#include "../NetworkLibrary/MyProtocol.h"
#include "UIFunctorFactory.h"

CommonApp* CommonApp::m_pInstance = nullptr;
HWND CommonApp::m_hWnd;

LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return CommonApp::m_pInstance->WndProc(hWnd, message, wParam, lParam);
}

CommonApp::CommonApp(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_szWindowClass(L"DefaultWindowClass")
    , m_szTitle(L"CommonApp")
{
    CommonApp::m_pInstance = this;

    m_wcex.hInstance = hInstance;
    m_wcex.cbSize = sizeof(WNDCLASSEX);
    m_wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    m_wcex.lpfnWndProc = DefaultWndProc;
    m_wcex.cbClsExtra = 0;
    m_wcex.cbWndExtra = 0;
    //m_wcex.hCursor = LoadCursorFromFile(L"../Resources/Textures/cursor1.ico");
    m_wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    m_wcex.lpszClassName = m_szWindowClass;
    m_pD3DRenderer = std::make_shared<D3DRenderManager>(ScreenWidth, ScreenHeight);

    //SetCursor(m_wcex.hCursor);
}

bool CommonApp::Initialize()
{
    // ���
    RegisterClassExW(&m_wcex);
    
    RECT rcClinet = { 0, 0, ScreenWidth, ScreenHeight };
    //AdjustWindowRect(&rcClinet, WS_OVERLAPPEDWINDOW, TRUE);

    // ������ ȭ�� �߾ӿ� ����
    const int screenCenterX = GetSystemMetrics(SM_CXSCREEN) / 2;
    const int screenCenterY = GetSystemMetrics(SM_CYSCREEN) / 2;
    const int startX = screenCenterX - ScreenWidth / 2;
    const int startY = screenCenterY - ScreenHeight / 2;

    // ������ ����
    m_hWnd = CreateWindowW(m_szWindowClass, m_szTitle, WS_OVERLAPPEDWINDOW,
        startX, startY,                       // ���� ��ġ
        rcClinet.right - rcClinet.left, rcClinet.bottom - rcClinet.top,                     // ����, ����
        nullptr, nullptr, m_hInstance, nullptr);

    if (!m_hWnd)
    {
        return false;
    }

    SetForegroundWindow(m_hWnd);    // �����츦 �ֻ�����
    SetFocus(m_hWnd);               // �����쿡 ��Ŀ���� �ش�

    // ������ ���̱�
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    // Renderer �ʱ�ȭ
    HR_T(m_pD3DRenderer->Initialize(m_hWnd));

    m_pDevice = m_pD3DRenderer->GetDevice();
    m_pDeviceContext = m_pD3DRenderer->GetDevcon();
    m_pSwapChain = m_pD3DRenderer->GetSwapChain();

    // todo ä�� : �������� ���� ���� �ٲ��ֱ�
    // �� ����� & �� �����ϱ� ���忡�� �����ϰ� �ϱ�
    if(m_bUseNetwork)
    {
        if (m_bHost)
        {
            m_server.Start(m_ip.c_str());
            // host�ΰ� session�� �˷��ֱ�
            char host[1] = { '1' };
            WorldManager::GetInstance()->PushSendQueue(
                WorldManager::GetInstance()->SerializeBuffer(sizeof(PacketC2S_IamHost), C2S_IAM_HOST_MSG, host),
                sizeof(PacketC2S_IamHost));
            // host�ΰ� WorldManager�� �˰� �ֱ�
            WorldManager::GetInstance()->SetHostServer();
        }
        m_client.Start(m_ip.c_str());
    }

    m_pTimeManager = std::make_unique<TimeManager>();
    PhysicsManager::GetInstance()->Initailize();
    InputManager::GetInstance()->Initialize();
    SoundManager::GetInstance()->Initailize();
    return true;
}

void CommonApp::Run()
{
    if (!m_bUseNetwork)
    {
        GameLoop();
    }

    // todo ä�� : ������ ���� �� �� ���ӷ��� �ٸ��� ������

    // host�� ��
    else if (m_bHost)
    {
        std::thread ServerThread([this]() { m_server.ServerLoop(); });
        std::thread ClientThread([this]() { m_client.ClientLoop(); });

        GameLoop();

        m_client.Stop();
        m_server.Stop();

        if (ServerThread.joinable())
            ServerThread.join();

        if (ClientThread.joinable())
            ClientThread.join();
    }
    // client�� ��
    else if (!m_bHost)
    {
        std::thread ClientThread([this]() { m_client.ClientLoop(); });

        GameLoop();

        m_client.Stop();

        if (ClientThread.joinable())
            ClientThread.join();
    }
}

void CommonApp::GameLoop()
{
    while (m_bRun)
    {
        if (PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))
        {
            if (m_msg.message == WM_QUIT)
                break;
            if (!TranslateAccelerator(m_msg.hwnd, m_hAccelTable, &m_msg))
            {
                TranslateMessage(&m_msg);
                DispatchMessage(&m_msg);
            }
        }
        else
        {
            Update();
            Render();
        }
    }
}

void CommonApp::Update()
{
    m_pTimeManager->Update();
    m_deltaTime = m_pTimeManager->GetDeltaTime();

    PhysicsManager::GetInstance()->Update(m_deltaTime);
    InputManager::GetInstance()->Update(m_deltaTime);
    SoundManager::GetInstance()->Update(m_deltaTime);

    m_pD3DRenderer->Update(m_deltaTime);

//#ifdef ENGINE_DEBUG
    ShowFrameRate();
//#endif
}

void CommonApp::Render()
{
    m_pD3DRenderer->Render();

    // �Ŵ��� ������

}

void CommonApp::Finalize()
{
    WorldManager::GetInstance()->Finalize();
    EventManager::GetInstance()->Finalize();
    PhysicsManager::GetInstance()->Finalize();
    InputManager::GetInstance()->Finalize();
    SoundManager::GetInstance()->Finalize();
    UIFunctorFactory::GetInstance()->Finalize();

    if (m_bHost)
        m_server.CleanUp();
    m_client.CleanUp();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CommonApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
	    return true;
	}
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_ACTIVATE:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
    case WM_KEYDOWN:
    {
        //if (wParam == VK_UP) {
        //    D3DRenderManager::Instance->IncreaseStaticModel("../Resources/FBX/zeldaPosed001.fbx");
        //    D3DRenderManager::Instance->IncreaseSkeletalModel("../Resources/FBX/SkinningTest.fbx");
        //    D3DRenderManager::Instance->IncreaseSkeletalModel("../Resources/FBX/Zombie_Run.fbx");
        //}
        //else if (wParam == VK_DOWN) {
        //    D3DRenderManager::Instance->DecreaseModel();
        //}
        //else if (wParam == VK_RIGHT)
        //{

        //}
    }
   // break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;
	case WM_SYSKEYDOWN:
		Keyboard::ProcessMessage(message, wParam, lParam);
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
		}
		break;
	case WM_MOUSEACTIVATE:
		// ���콺 Ŭ������ ������ Ȱ��ȭ �� ��, ���콺 Ŭ�� �Է� ����
		return MA_ACTIVATEANDEAT;
    
    
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void CommonApp::ShowFrameRate()
{
    float fps = m_pTimeManager->GetFrameRate();
    std::wstring windowText;
    windowText.append(m_szTitle);
    windowText.append(L"   FPS: ");
    windowText.append(std::to_wstring(fps));
    SetWindowText(m_hWnd, windowText.c_str());
}
