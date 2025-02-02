#include "pch.h"
#include "UIMeshComponent.h"

#include "CommonApp.h"
#include "../D3DRenderer/SkeletalMeshModel.h"
#include "../D3DRenderer/Material.h"
#include "../D3DRenderer/Helper.h"

UIMeshComponent::~UIMeshComponent()
{
	m_skeletalMeshModel.reset();
	m_animationMap.clear();
}

Material* UIMeshComponent::SetMaterial(Material* material, int instanceNumber)
{
	Material* prevMaterial = m_skeletalMeshModel->GetMaterial(instanceNumber);
	m_skeletalMeshModel->SetInstanceMaterial(material, instanceNumber);
	return prevMaterial;
}

void UIMeshComponent::SetOutLine(bool isOutLine)
{
	m_skeletalMeshModel->SetIsOutLine(isOutLine);
}

void UIMeshComponent::SetUIMove(bool isUIMove)
{
	m_skeletalMeshModel->SetIsMove(isUIMove);
}

void UIMeshComponent::SetDefaultAnimation(std::string_view filePath, bool isLoop)
{
	m_filePath = filePath;
	m_isLoop = isLoop;

	m_skeletalMeshModel = std::make_shared<SkeletalMeshModel>();
	m_skeletalMeshModel->SetWorldMatrix(&m_worldMatrix);
	m_skeletalMeshModel->SetAnimationTime(&m_animationProgressTime);
	m_skeletalMeshModel->ReadSceneResourceFromFBX(m_filePath);
	m_skeletalMeshModel->m_isLoop = &m_isLoop;
	m_animationMap.emplace(m_filePath, m_animationMap.size());

	m_durationTime = m_skeletalMeshModel->GetAnimationDuration(m_animationIndex);
}

void UIMeshComponent::ChangeAnimation(std::string name)
{
	// ���� �ִϸ��̼� index
	m_animationIndex = m_animationMap.find(name)->second;

	// ���� index ������ ��忡 �ִϸ��̼� ���ε�
	m_skeletalMeshModel->ChangeBoneAnimationReference(m_animationIndex);

	// �ִϸ��̼ǿ� �ش���� ���� Bone�� localTrnasform �ʱ�ȭ
	m_skeletalMeshModel->SettingBindposeMatrix();

	// �ִϸ��̼� �ٲ���ٴ� Ʈ����
	m_bAnimChanged = true;
}

void UIMeshComponent::AddAnimation(std::string filePath, bool isLoop)
{
	m_skeletalMeshModel->AddSceneAnimationFromFBX(filePath, isLoop);
	m_animationMap.emplace(filePath, m_animationMap.size());
}

void UIMeshComponent::Initialize()
{
	m_skeletalMeshModel->PlayAnimation(m_animationIndex);
	m_durationTime = m_skeletalMeshModel->GetAnimationDuration(m_animationIndex);
	RenderComponent::Initialize();
}

void UIMeshComponent::Update(float deltaTime)
{
	RenderComponent::Update(deltaTime);

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

	if (m_bVisible)
		CommonApp::m_pInstance->GetRenderer()->AddUISkeletalMeshModel(m_skeletalMeshModel, m_PlayerNumber, m_skeletalMeshModel->m_IsOutLine);
}
