#pragma once
#include "../Engine/CollisionHandler.h"
class ReadyCollisionHandler :
    public CollisionHandler
{
    // CollisionHandler��(��) ���� ��ӵ�
    void OnContactEnter(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxContactPair& cp) override;
    void OnContactStay(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxContactPair& cp) override;
    void OnContactExit(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxContactPair& cp) override;
    void OnTriggerEnter(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxTriggerPair& tp) override;
    void OnTriggerExit(std::weak_ptr<GameObject> actor1, std::weak_ptr<GameObject> actor2, const physx::PxTriggerPair& tp) override;
};

