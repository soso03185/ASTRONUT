#pragma once
#include "Component.h"
#include "InputManager.h"
#include "Controller.h"
#include <type_traits>
/// <summary>
/// ControllerComponent
/// ���� �ʿ��� ���ӿ�����Ʈ���� �޾��ָ� ��.
/// �ް� CreateController<�װ� ���� ���� ��ǲ ó�� ���� Ŭ����>()ȣ�����ָ� �ȴ�.
/// �װ� ���� ���� ��ǲ ó�� ���� Ŭ������ Controller Ŭ���� ��ӽ��Ѽ� ���� ����~
/// ���� Ȱ��ȭ �� �ִ� ���忡 �ִ� ������Ʈ�� ControllerComponent�� ��ǲó���� ������Ʈ ���ư�.
/// </summary>

class ControllerComponent :
    public Component , public InputProcesser
{
public:
    template <class T>
        requires std::is_base_of<Controller, T>::value
    void CreateController();

    void ClearControllers();

    virtual void OnInputProcess(
        const Keyboard::State& KeyState,
        const Keyboard::KeyboardStateTracker& KeyTracker,
        const Mouse::State& MouseState,
        const Mouse::ButtonStateTracker& MouseTracker) final;

public:
    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;

public:
    ~ControllerComponent();

private:
    bool m_IsUpdate = false;
    std::list<std::unique_ptr<Controller>> m_InputOperations;
};

template<class T>
    requires std::is_base_of<Controller, T>::value
inline void ControllerComponent::CreateController()
{
    std::unique_ptr<Controller> controller = std::make_unique<T>();
    controller->SetOwner(GetOwner());
    m_InputOperations.push_back(std::move(controller));
}
