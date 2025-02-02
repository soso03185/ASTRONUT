#include "pch.h"
#include "Bone.h"
#include "ModelResource.h"
#include "NodeAnimation.h"
// #include "TutorialApp.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp/scene.h>

#include "SkeletalMeshModel.h"

namespace Math = DirectX::SimpleMath;

void Bone::Update(float deltaTime)
{
	if (m_pCurNodeAnimation != nullptr)
	{
		Math::Matrix transform = m_pCurNodeAnimation->Evaluate(*m_pAnimationTime);
		m_localTransform = transform;

		// �ִϸ��̼� ��ȯ�� ��Ȳ�� ���
		if (m_pNextNodeAnimation != nullptr)
		{
			m_AnimChangeTime += deltaTime;

			float changingAnimTime = 0.2f;

			// ��ȯ �� �ִϸ��̼��� Ű ���� Matrix�� ��������
			Math::Matrix nextTransform = m_pNextNodeAnimation->Evaluate(*m_pAnimationTime);
			m_localTransform = Math::Matrix::Lerp(transform, nextTransform, m_AnimChangeTime * 5);

			if (m_AnimChangeTime >= changingAnimTime)
			{
				m_pCurNodeAnimation = m_pNextNodeAnimation;
				m_pNextNodeAnimation = nullptr;
				m_AnimChangeTime = 0;
			}
		}
	}

	if (m_pParentBone != nullptr)
		m_worldTransform = m_localTransform * m_pParentBone->m_worldTransform;
	else
		m_worldTransform = m_localTransform * *m_pParentModel->m_worldMatrix;

	for (auto& child : m_Children)
	{
		child.Update(deltaTime);
	}
}

Bone* Bone::FindNode(const std::string& name)
{
	if (m_name == name)
		return this;

	for (auto& child : m_Children)
	{
		Bone* found = child.FindNode(name);
		if (found != nullptr)
			return found;
	}

	return nullptr;
}

Bone& Bone::CreateChild()
{
	return m_Children.emplace_back();
}

