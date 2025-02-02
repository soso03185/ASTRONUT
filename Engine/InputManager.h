#pragma once
#include <directXTK/Mouse.h>
#include <directXTK/Keyboard.h>
#include "Singleton.h"

using namespace DirectX;
/// <summary>
/// InputManager
/// Input �ʿ��� Ŭ��������
/// InputProcesser ��ӽ�Ű��,
/// InputManager::GetInstance()->AddInputProcesser(this); �ѹ� ȣ���ϰ�,
/// OnInputProcess() �������ָ� ��.
/// ������ �ʿ��� �����ڷ�
/// https://github.com/Microsoft/DirectXTK/wiki/Keyboard
/// https://github.com/Microsoft/DirectXTK/wiki/Mouse
/// </summary>

class InputProcesser
{
public:
	virtual void OnInputProcess(
		const Keyboard::State& KeyState,
		const Keyboard::KeyboardStateTracker& KeyTracker,
		const Mouse::State& MouseState,
		const Mouse::ButtonStateTracker& MouseTracker
	) abstract;
};

class InputManager
	: public Singleton<InputManager>
{
public:
	Keyboard* GetKeyboard() const { return m_Keyboard.get(); }
	Mouse* GetMouse() const { return m_Mouse.get(); }

public:
	void AddInputProcesser(InputProcesser* inputProcesser);
	void RemoveInputProcesser(InputProcesser* inputProcesser);

public:
 	void Initialize();
	void Finalize();
	void Update(float DeltaTime);

private:
	std::list<InputProcesser*> m_InputProcessers;

	std::unique_ptr<Keyboard> m_Keyboard;
	std::unique_ptr<Mouse> m_Mouse;

	Keyboard::KeyboardStateTracker m_KeyboardStateTracker;
	Mouse::ButtonStateTracker m_MouseStateTracker;

	Mouse::State m_MouseState;
	Keyboard::State m_KeyboardState;
};

