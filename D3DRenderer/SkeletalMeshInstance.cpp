#include "pch.h"
#include "SkeletalMeshInstance.h"
#include "Mesh.h"
#include "Bone.h"

SkeletalMeshInstance::SkeletalMeshInstance()
{

}

SkeletalMeshInstance::~SkeletalMeshInstance()
{

}

void SkeletalMeshInstance::Create(Mesh* pMeshResource, Bone* pRootNode, Material* pMaterial)
{
	m_pMeshResource = pMeshResource;
	m_pMaterial = pMaterial;

	size_t meshBoneCount = m_pMeshResource->m_BoneReferences.size();	// �޽��� ����� ������
	m_BoneReferences.resize(meshBoneCount); // �� ���� ���� �����̳� ũ�� ����		
	for (size_t i = 0; i < meshBoneCount; ++i)
	{
		std::string& name = m_pMeshResource->m_BoneReferences[i].NodeName;
		Bone* pNode = pRootNode->FindNode(name);
		assert(pNode != nullptr);
		m_BoneReferences[i] = &pNode->m_worldTransform;
	}
	m_pNodeWorldTransform = &pRootNode->m_worldTransform;
}

void SkeletalMeshInstance::BindingMaterial(Material* pMaterial)
{
	m_pMaterial = pMaterial;
}

void SkeletalMeshInstance::UpdateMatrixPallete(Math::Matrix* MatrixPalettePtr)
{
	assert(m_BoneReferences.size() == m_pMeshResource->m_BoneReferences.size());
	size_t meshBoneCount = m_pMeshResource->m_BoneReferences.size();	// �޽��� ����� ������
	for (size_t i = 0; i < meshBoneCount; ++i)
	{
		Math::Matrix& BoneNodeWorldMatrix = *m_BoneReferences[i];

		// HLSL ������ۿ� ������Ʈ�Ҷ� �ٷ� �����Ҽ��ֵ��� ��ġ�ؼ� �����Ѵ�.
		BoneReference& br = m_pMeshResource->m_BoneReferences[i];
		MatrixPalettePtr[br.BoneIndex] = (br.OffsetMatrix * BoneNodeWorldMatrix).Transpose();
	}
}

void SkeletalMeshInstance::Render(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetIndexBuffer(m_pMeshResource->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetVertexBuffers(0, 1, m_pMeshResource->m_pBWVertexBuffer.GetAddressOf(), &m_pMeshResource->m_VertexBufferStride, &m_pMeshResource->m_VertexBufferOffset);
	deviceContext->DrawIndexed(m_pMeshResource->m_IndexCount, 0, 0);
}