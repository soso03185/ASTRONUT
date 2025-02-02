#pragma once
#include "UIClickFunctor.h"
class UIClickPopUpFunctor :
    public UIClickFunctor
{
public:
    // UIClickFunctor��(��) ���� ��ӵ�
    void Execute(UIObject& onwer, float deltaTime) override;

    void SetTargetUI(std::weak_ptr<UIObject> targetUI);

private:
    std::weak_ptr<UIObject> m_TargetUI;
};

