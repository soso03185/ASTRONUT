#pragma once

#include "GameObject.h"
#include "UIManager.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <cassert>

enum class eWorldType
{
	MAIN,
	STAGE1,
	STAGE2,
	STAGE3,
	STAGE4,
	STAGE5,

	//TEST,
	//TEST2,
	//TEST3,
	STARTCUTSCECNE,
	STARTFREEWORLD,
	ENDCUTSCENE,
	ENDFREEWORLD,
	CREDIT,
	GAMEOVERWORLD,

	END
};


class StaticMeshModel;
class SkeletalMeshModel;

/// <summary>
/// World�� ���� Ŭ����
/// GameApp�� World �Ŵ����� ���� World���� �����Ѵ�.
///	Init, Finalize�� OnEnter, OnExit�� ���еǾ� �ִ�.
///	���� ���������� world->init, finalize�� ���� �� ��
/// World�� GameObject ����Ʈ�� ��� �ִ�.
/// </summary>
class World
	: public std::enable_shared_from_this<World>
{
public:
	World();
	virtual ~World();

public:
	// GetGameObjects�� ���� ���� ���͸� ������� ������ ����Ͽ� ��ü ���� ȥ���� �ָ� �ȵȴ�.
	std::vector<std::shared_ptr<GameObject>>& GetGameObjects(eObjectType type) { return m_gameObjects[static_cast<int>(type)]; }
	std::shared_ptr<GameObject> GetGameObject(std::string_view name);
	std::string GetName() const { return m_name; }

	void SetWorldType(eWorldType worldType) { m_worldType = worldType; }
	void SetName(std::string_view name) { m_name = name; }

	std::shared_ptr<UIManager> GetUIManager() { return m_UIManager; }

public:
	void DeleteGameObject(std::string_view name);
	void DeleteGameObject(const std::shared_ptr<GameObject> gameObj);

	virtual void Update(float deltaTime);

	/// Init, Finalize �̿ܿ� ���� ��ȯ�� �ʿ��� �߰� �ൿ
	virtual void OnEnter();
	virtual void OnExit();

private:
	// �̸� �ߺ� Ȯ��
	std::string IsRegisteredName(std::string_view name);

protected:
	/// ������Ʈ ����, �ı�
	virtual void Initialize();
	virtual void Finalize();


protected:
	std::string m_name;
	eWorldType m_worldType = eWorldType::END;

	std::vector<std::shared_ptr<GameObject>> m_gameObjects[static_cast<int>(eObjectType::END)];
	std::unordered_map<std::string, std::pair<eObjectType, int>> m_objectMap;
	std::shared_ptr<UIManager> m_UIManager;

public:
	template <typename T>
	std::weak_ptr<T> CreateGameObject(std::string_view objName, eObjectType objectType)
	{
		std::shared_ptr<T> obj = std::make_shared<T>();
		GameObject* inputObj = static_cast<GameObject*>(obj.get());
		assert(inputObj);

		std::string newName = IsRegisteredName(objName);
		obj->SetName(newName);
		obj->SetObjectType(objectType);
		obj->SetOwnerWorld(shared_from_this());

		int keyIdx = m_gameObjects[static_cast<int>(objectType)].size();
		m_objectMap.emplace(newName, std::make_pair(objectType, keyIdx));

		m_gameObjects[static_cast<int>(objectType)].emplace_back(obj);

		return obj;
	}

	template <typename T>
	std::weak_ptr<T> CreateGameObjectRuntime(std::string_view objName, eObjectType objectType)
	{
		std::shared_ptr<T> obj = std::make_shared<T>();
		GameObject* inputObj = static_cast<GameObject*>(obj.get());
		assert(inputObj);

		std::string newName = IsRegisteredName(objName);
		obj->SetName(newName);
		obj->SetObjectType(objectType);
		obj->SetOwnerWorld(shared_from_this());

		int keyIdx = m_gameObjects[static_cast<int>(objectType)].size();
		m_objectMap.emplace(newName, std::make_pair(objectType, keyIdx));

		m_gameObjects[static_cast<int>(objectType)].emplace_back(obj);

		obj->Initialize();

		return obj;
	}
};

