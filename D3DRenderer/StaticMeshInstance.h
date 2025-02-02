#pragma once

class Mesh;
class Material;

// ���������� �׷����� Ŭ����.
class StaticMeshInstance 
{
public:
	StaticMeshInstance();
	~StaticMeshInstance();

public:
	Mesh* m_pMeshResource = nullptr;
	Material* m_pMaterial = nullptr;
	Math::Matrix* m_pNodeWorldTransform = nullptr;		// StaticMesh�� ��������� ���� ����� ������
	BoundingBox* m_pBoundingBox;

	void Create(Mesh* pMeshResource, Math::Matrix* pWorld, Material* pMaterial, BoundingBox* pboundingBox);
	virtual void Render(ID3D11DeviceContext* deviceContext);
};
