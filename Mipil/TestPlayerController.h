#pragma once
#include "../Engine/Controller.h"
#include "PxPhysicsAPI.h"

class TestWorld;
class TestPlayerObject;

class TestPlayerController :
    public Controller
{
public:
    // Controller��(��) ���� ��ӵ�
    void HandleInput(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker, const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker) override;
    void Update(float deltaTime) override;

private:
    TestWorld* m_world;
    TestPlayerObject* m_player;
    physx::PxRigidDynamic* m_rigidDynamic;
};

