#pragma once

#include <memory>
#include <unordered_map>

#include "Object.h"
#include "Component.h"

class World;
class Component;
class SceneComponent;

enum class eObjectType
{
	TEST,
	PLANE,
	MAP,
	LEVEL,
	HOLLOWBOX,
	PART,
	PLAYER,
	PARTICLE,
	CAMERA,
	EMPTY,
	READYTRIGGER,

	END
};

/// <summary>
/// GameObject���� ����
/// ���ӿ�����Ʈ ��ü�δ� �ƹ��͵� ������ �ʴ´�.
/// ������Ʈ �� ��ǥ ������Ʈ�� �����Ͽ� �ش� ������Ʈ�� ��ġ���� ��ǥ��ġ������ ������.
/// </summary>
class GameObject
	: public Object
	, public std::enable_shared_from_this<GameObject>
{
public:
	GameObject() = default;
	virtual ~GameObject();

public:
	void SetName(std::string_view name) { m_name = name; }
	void SetObjectType(eObjectType type) { m_objectType = type; }
	void SetOwnerWorld(std::shared_ptr<World> pWorld) { m_pOwnerWorld = pWorld; }
	void SetParentObject(std::shared_ptr<GameObject> pObject) { m_pParentObj = pObject; }
	void ResetParentObject() { m_pParentObj.reset(); }
	void SetRootComponent(std::shared_ptr<SceneComponent> pRootcomponent) { m_pRootComponent = pRootcomponent; }
	void SetPosition(Vector3 position);
	void SetRotation(Vector3 rotation);
	void SetScale(Vector3 scale);
	void SetDeferredPosition(Vector3 position) { m_deferredPosition = position; }
	void SetDeferredRotation(Vector3 rotation) { m_deferredRotation = rotation; }

	std::string& GetName() { return m_name; }
	eObjectType GetObjectType() const { return m_objectType; }
	std::weak_ptr<World> GetOwnerWorld() const { return m_pOwnerWorld; }
	std::weak_ptr<GameObject> GetParentObject() const { return m_pParentObj; }
	std::weak_ptr<SceneComponent> GetRootComponent() const { return m_pRootComponent; }
	std::weak_ptr<Component> GetComponent(std::string_view name) const;
	const Vector3& GetLocalPosition() const;
	const Vector3& GetLocalRotation() const;
	const Vector3& GetLocalScale() const;
	const Vector3& GetWorldPosition() const;
	const Vector3& GetWorldRotation() const;
	const Vector3& GetWorldScale() const;
	const Matrix& GetTransform() const;

public:
	virtual void Initialize();
	virtual void Update(float deltaTime);


protected:
	std::string m_name;
	eObjectType m_objectType = eObjectType::END;

	std::weak_ptr<World> m_pOwnerWorld;
	std::weak_ptr<GameObject> m_pParentObj;

	std::unordered_map<std::string, int> m_componentMap; // GetComponent �ӵ� ����� ���� Query�� �ڷᱸ��
	std::vector<std::shared_ptr<Component>> m_ownComponents;
	std::shared_ptr<SceneComponent> m_pRootComponent;	// m_pRootComponent�� m_OwnedComponent �� �ϳ��� ����, ��ǥ ��ġ��

	Vector3 m_deferredPosition; //SceneComponent Create�ϱ� ���� �ۿ��� SetPosition ���� �� ���;
	Vector3 m_deferredRotation; //SceneComponent Create�ϱ� ���� �ۿ��� SetRotation ���� �� ���;

public:
	template <typename T>
	std::weak_ptr<T> CreateComponent(std::string_view name)
	{
		std::shared_ptr<T> component = std::make_shared<T>();
		Component* inputComponent = dynamic_cast<Component*>(component.get());
		assert(inputComponent);

		component->SetName(name);
		component->SetOwner(shared_from_this());

		int keyIdx = m_ownComponents.size();
		m_componentMap.emplace(name, keyIdx);

		m_ownComponents.emplace_back(component);

		return component;
	}

	template <typename T>
	std::weak_ptr<T> GetComponentByTypeName() const
	{
		for (const auto& component : m_ownComponents)
		{
			std::shared_ptr<T> typedComponent = std::dynamic_pointer_cast<T>(component);
			if (typedComponent)
			{
				return typedComponent;
			}
		}
		return std::weak_ptr<T>();
	}
};