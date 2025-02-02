#pragma once
#include "PxPhysicsAPI.h"
/// <summary>
/// CollisionHandler
/// �浹 ���� �����ϴ� ���� ���� Ŭ����
/// �̰� ��ӽ�Ų Ŭ�������� �� �Լ� ���� ���ְ�
/// ������ٵ� ������Ʈ���� AddCollisionHandler�� �ٿ��ָ� ��.
/// </summary>
class GameObject;

class CollisionHandler
{
public:
	virtual ~CollisionHandler() {}

	virtual void OnContactEnter(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxContactPair& cp) abstract;
	virtual void OnContactStay(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxContactPair& cp) abstract;
	virtual void OnContactExit(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxContactPair& cp) abstract;

	virtual void OnTriggerEnter(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxTriggerPair& tp) abstract;
	virtual void OnTriggerExit(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxTriggerPair& tp) abstract;


};

