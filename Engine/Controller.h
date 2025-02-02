#pragma once
#include <directXTK/Mouse.h>
#include <directXTK/Keyboard.h>
#include "GameObject.h"

using namespace DirectX;
/// <summary>
/// Controller
/// �̰� ��ӽ��Ѽ� ���� �뵵�� �´� ��ǲ ó�� ���� ���� ���ָ� �ȴ�~
/// HandleInput()�� �� Ű �Է¿� ���� ���� ���� ó���ϰ�,
/// Run()���� �� ó���� �����ͷ� ������Ʈ ���ٰ� ������Ʈ ���ָ�
/// HandleInput()�� ��ǲ�Ŵ��� ������Ʈ �� �� ó���ǰ�,
/// Run()�� ����->���� ������Ʈ->ControllerComponent ������Ʈ �� �� ȣ��ȴ�.
/// ���⼭ ó���� �����ͷ� GameObject�� �����ֱ� ���� �Ϸ���
/// ControllerComponent->CreateController() �� �� m_owner����
/// ControllerComponent�� ���� GameObject �־���. �̰ɷ� �����ؼ� ó���ϼ�.
/// </summary>

class Controller
{
public:
	virtual void HandleInput(
		const Keyboard::State& KeyState,
		const Keyboard::KeyboardStateTracker& KeyTracker,
		const Mouse::State& MouseState,
		const Mouse::ButtonStateTracker& MouseTracker) abstract;
	virtual void Update(float deltaTime) abstract;

	void SetOwner(std::weak_ptr<GameObject> owner) { m_owner = owner; }

protected:
	std::weak_ptr<GameObject> m_owner;
};

