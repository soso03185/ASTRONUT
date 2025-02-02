#include "pch.h"
#include "SkeletalMeshComponent.h"

#include "CommonApp.h"
#include "../D3DRenderer/SkeletalMeshModel.h"
#include "../D3DRenderer/Material.h"
#include "../D3DRenderer/ResourceManager.h"

SkeletalMeshComponent::~SkeletalMeshComponent()
{
	m_skeletalMeshModel.reset();
	m_animationMap.clear();
}

void SkeletalMeshComponent::SetIsOutLine(bool isOutLine)
{
	m_skeletalMeshModel->SetIsOutLine(isOutLine);
}

bool SkeletalMeshComponent::GetIsOutLine()
{
	return m_skeletalMeshModel->m_IsOutLine;
}

Material* SkeletalMeshComponent::SetMaterial(Material* material, int instanceNumber)
{
	Material* prevMaterial = m_skeletalMeshModel->GetMaterial(instanceNumber);
	m_skeletalMeshModel->SetInstanceMaterial(material, instanceNumber);
	return prevMaterial;
}

void SkeletalMeshComponent::SetDefaultAnimation(std::string_view filePath, bool isLoop)
{
	m_filePath = filePath;
	m_isLoop = isLoop;

	m_skeletalMeshModel = std::make_shared<SkeletalMeshModel>();
	m_skeletalMeshModel->SetWorldMatrix(&m_relativeMatrix);
	m_skeletalMeshModel->SetAnimationTime(&m_animationProgressTime);
	m_skeletalMeshModel->ReadSceneResourceFromFBX(m_filePath);
	m_skeletalMeshModel->m_isLoop = &m_isLoop;
	m_animationMap.emplace(m_filePath, m_animationMap.size());

	m_durationTime = m_skeletalMeshModel->GetAnimationDuration(m_animationIndex);
}

void SkeletalMeshComponent::ChangeAnimation(std::string name)
{
	// ���� �ִϸ��̼� index
	m_animationIndex = m_animationMap.find(name)->second;

	// ���� index ������ ��忡 �ִϸ��̼� ���ε�
	m_skeletalMeshModel->ChangeBoneAnimationReference(m_animationIndex);

	// �ִϸ��̼ǿ� �ش���� ���� Bone�� localTrnasform �ʱ�ȭ
	m_skeletalMeshModel->SettingBindposeMatrix();

//	m_isLoop = �ٲ�� �ִϸ��̼��� �������� ;

	// �ִϸ��̼� �ٲ���ٴ� Ʈ����
	m_bAnimChanged = true;

}

void SkeletalMeshComponent::AddAnimation(std::string filePath, bool animLoop)
{
	m_skeletalMeshModel->AddSceneAnimationFromFBX(filePath, animLoop);
	m_animationMap.emplace(filePath, m_animationMap.size());
}

void SkeletalMeshComponent::Initialize()
{
	m_skeletalMeshModel->PlayAnimation(m_animationIndex);
	m_durationTime = m_skeletalMeshModel->GetAnimationDuration(m_animationIndex);
	RenderComponent::Initialize();
}

void SkeletalMeshComponent::Update(float deltaTime)
{
	RenderComponent::Update(deltaTime);
	SetOffsetMatrix();

	if (!m_animationMap.empty())
	{
		m_animationProgressTime += deltaTime * m_animationPlaySpeed;

		// for DurationTime deffered change
		if (m_bAnimChanged == false && m_isLoop == true)
		{
			m_animationProgressTime = (float)fmod(m_animationProgressTime, m_durationTime);
		}
		else if (m_bAnimChanged == true)
		{
			if (m_animationProgressTime >= m_durationTime)
			{
				m_durationTime = m_skeletalMeshModel->GetAnimationDuration(m_animationIndex);
				m_bAnimChanged = false;
			}
		}
	}

	m_skeletalMeshModel->UpdateBoneNode(deltaTime);

	CommonApp::m_pInstance->GetRenderer()->AddSkeletalMeshModel(m_skeletalMeshModel);
}

void SkeletalMeshComponent::SetOffsetMatrix()
{
	if (m_offsetPosition != Math::Vector3(0.f, 0.f, 0.f) ||
		m_offsetRotation != Math::Vector3(0.f, 0.f, 0.f))
	{
		m_offsetMatrix = Math::Matrix::CreateFromYawPitchRoll(XMConvertToRadians(m_offsetRotation.y), XMConvertToRadians(m_offsetRotation.x), XMConvertToRadians(m_offsetRotation.z)) * Math::Matrix::CreateTranslation(m_offsetPosition);

		m_relativeMatrix = m_worldMatrix * m_offsetMatrix;
	}
	else
	{
		m_relativeMatrix = m_worldMatrix;
	}
}
