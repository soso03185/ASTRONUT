#include "pch.h"
#include "MaterialManagerComponent.h"

#include "SkeletalMeshComponent.h"
#include "UIMeshComponent.h"

#include "../D3DRenderer/SkeletalMeshModel.h"
#include "../D3DRenderer/ResourceManager.h"

using namespace std;

MaterialManagerComponent::~MaterialManagerComponent()
{
}

void MaterialManagerComponent::Initialize()
{
	m_MeshInstanceNumber = 0;
	m_bIsComeBack = false;
	m_ComebackTime = 0.f;
	m_DurationTime = 0.f;
}

void MaterialManagerComponent::Update(float deltaTime)
{
	m_DurationTime += deltaTime;

	// Material�� Comeback�ؾ� �Ѵٸ�
	// �Է��� �ð��� ������ Previous Material���� �ٽ� �����ϱ�
	if (m_bIsComeBack && m_ComebackTime <= m_DurationTime)
	{
		if (m_SkeletalMeshComponent.lock())
		{
			m_SkeletalMeshComponent.lock()->SetMaterial(m_PreviousMaterial, m_MeshInstanceNumber);
			m_SkeletalMeshComponent.lock() = nullptr;
		}

		if (m_UIMeshComponent.lock())
		{
			m_UIMeshComponent.lock()->SetMaterial(m_PreviousMaterial, m_MeshInstanceNumber);
			m_UIMeshComponent.lock() = nullptr;
		}
	}
}


/// 3D UI Mesh�� Material �����ϱ� ///
void MaterialManagerComponent::ChangedMaterial(std::weak_ptr<SkeletalMeshComponent> meshComponent, const std::string& changedMaterialName, int instanceNumber, bool isComeback, float comebackTime)
{
	m_MeshInstanceNumber = instanceNumber;
	m_bIsComeBack = isComeback;
	m_ComebackTime = comebackTime;
	m_DurationTime = 0.f;

	m_SkeletalMeshComponent = meshComponent;
	m_CurrentMaterial = ResourceManager::GetInstance()->CreateMaterial(changedMaterialName).get();

	if (m_PreviousMaterial == nullptr)
		m_PreviousMaterial = meshComponent.lock()->GetSkeletalMeshModel()->GetMaterial(instanceNumber);

	// �ش� �޽��� ������Ʈ�� ���͸����� �����ϸ� ������ ���͸����� ��ȯ�Ѵ�.
	m_SkeletalMeshComponent.lock()->SetMaterial(m_CurrentMaterial, m_MeshInstanceNumber);
}


/// 3D UI Mesh�� Material �����ϱ� ///
void MaterialManagerComponent::ChangedMaterial(std::weak_ptr<UIMeshComponent> meshComponent, const std::string& changedMaterialName, int instanceNumber, bool isComeback, float comebackTime)
{
	m_MeshInstanceNumber = instanceNumber;
	m_bIsComeBack = isComeback;
	m_ComebackTime = comebackTime;
	m_DurationTime = 0.f;

	m_UIMeshComponent = meshComponent;

	if (m_PreviousMaterial == nullptr)
		m_PreviousMaterial = meshComponent.lock()->GetSkeletalMeshModel()->GetMaterial(instanceNumber);

	m_CurrentMaterial = ResourceManager::GetInstance()->CreateMaterial(changedMaterialName).get();

	// �ش� �޽��� ������Ʈ�� ���͸����� �����ϸ� ������ ���͸����� ��ȯ�Ѵ�.
	m_UIMeshComponent.lock()->SetMaterial(m_CurrentMaterial, m_MeshInstanceNumber);
}


/// Material �����ϱ� ( Material �����ϰ� �ش� Material�� �ؽ��� ���� ) ///
void MaterialManagerComponent::MaterialSetting(const std::string& materialName, const std::wstring& texturePath, MaterialMapFlags materialTextureType)
{
	shared_ptr<Material> material = ResourceManager::GetInstance()->CreateMaterial(materialName);
	material->SetTexture(materialTextureType, texturePath);

}
