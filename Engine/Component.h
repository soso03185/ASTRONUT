#pragma once

#include "Object.h"

class GameObject;

/// <summary>
/// ���� ������ Component
/// Component�� ���� ������ �θ� ������Ʈ, �̸��� ������.
/// �뷫���� Component ������ �Ʒ��� ����.
/// Component
///   -SceneComponent
///     -CameraComponent
///     -RenderComponent
///       -RenderableComponents...
///   -MovementComponent
///   -FSMComponent
/// </summary>
class Component
	: public Object
{
public:
	Component() = default;
	virtual ~Component() = default;

public:
	void SetOwner(std::shared_ptr<GameObject> pOwner) { m_pOwner = pOwner; }
	void SetName(std::string_view name) { m_name = name; }

	std::weak_ptr<GameObject> GetOwner() const { return m_pOwner; }

public:
	virtual void Initialize() abstract;
	virtual void Update(float deltaTime) abstract;


protected:
	std::weak_ptr<GameObject> m_pOwner;
	std::string m_name;
};

