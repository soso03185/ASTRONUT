#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <memory>
#include <thread>

#include "TimeManager.h"

#include "../D3DRenderer/D3DRenderManager.h"
#include "../Engine/Server.h"
#include "../Engine/Client.h"

class D3DRenderManager;
class ResourceManager;

const int MAX_LOADSTRING = 100;

/// <summary>
/// ���ӿ��� ����� �������� ó���� �ϴ� class
/// ������ ���� ������ CommonApp�� ��� �޾� �ΰ����� ó���� �Ѵ�.
///	�̱��� �������� ����� Window, DirectX �ڿ��� ���� ������ �����ϰ� �Ѵ�.
/// </summary>
class CommonApp
{
protected:
	CommonApp(HINSTANCE hInstance);
	virtual ~CommonApp() = default;

public:
	static HWND GetHwnd() { return m_hWnd; }
	std::shared_ptr<D3DRenderManager> GetRenderer() const { return m_pD3DRenderer; }
	ComPtr<ID3D11Device> GetDevice() const { return m_pDevice; }
	ComPtr<ID3D11DeviceContext> GetDevcon() const { return m_pDeviceContext; }


public:
	virtual bool Initialize();
	void Run();
	void GameLoop();
	virtual void Update();
	virtual void Render();
	virtual void Finalize();

	void Exit() { m_bRun = false; }

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void ShowFrameRate();


public:
	static CommonApp* m_pInstance;	// �� ������ ���ƾ��ϱ� ������ public���� ����

protected:
	static HWND m_hWnd;

	HACCEL m_hAccelTable;
	MSG m_msg;
	HINSTANCE m_hInstance;
	WCHAR m_szTitle[MAX_LOADSTRING];
	WCHAR m_szWindowClass[MAX_LOADSTRING];
	WNDCLASSEXW m_wcex;
	int m_nCmdShow;

	FLOAT m_deltaTime;

/// Manager
	std::unique_ptr<TimeManager> m_pTimeManager;

// Network
	ServerNetworkManager m_server;
	ClientNetworkManager m_client;

	// ���� ȣ��Ʈ���� �ƴ���
	bool m_bHost = false;
	// �� ������
	std::string m_ip;
	// ��Ʈ��ũ ������ �� ������
	bool m_bUseNetwork = false;
	bool m_bRun = true;

/// D3DRenderer�� ����� �ڿ�
protected:
	std::shared_ptr<D3DRenderManager> m_pD3DRenderer = nullptr;

	ComPtr<ID3D11Device> m_pDevice = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
	ComPtr<IDXGISwapChain> m_pSwapChain = nullptr;
};

