#pragma once
#include "../Engine/UIClickPopUpFunctor.h"

class PlayerObject;

class UIReadyClickPopUpFunctor 
    : public UIClickPopUpFunctor
{
public:
    // UIClickFunctor��(��) ���� ��ӵ�
    void Execute(UIObject& onwer, float deltaTime) override;

    void SetPlayer(std::shared_ptr<PlayerObject> player) { m_player = player; }

private:
    std::weak_ptr<PlayerObject> m_player;
};