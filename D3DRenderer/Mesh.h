#pragma once

#include "D3DRenderManager.h"

using namespace std;

class SkeletonResource;

struct Vertex
{
	Vector4 Position;    // ���� ��ġ ����
	Vector2 TexCoord;
	Vector3 Normal;
	Vector3 Tangent;
};

// Skinning ���� vertex �̰� �ٸ� ���� ��� ���� �׳� vertex�� ���.
struct BoneWeightVertex
{
	Vector4 Position;    // ���� ��ġ ����
	Vector2 TexCoord;
	Vector3 Normal;
	Vector3 Tangent;

	// �����ϴ� ���� �ε����� ������ �ִ� 128���� �ϴ� ó������. 
	// ���ؽ��� �����ϴ� ���� �ִ� 4���� �����ߴµ� �� ���� ���� Index
	int BlendIndeces[4] = {};  // ����޴� �� ���� �ִ� 4���� �����Ѵ�.

	// ���ؽ����� �����ϴ� ���� weight ��.
	// �ִ� 4���� �����ϴϱ� weight ���� 4������ �ְ� ��.
	float BlendWeights[4] = {};  // ����ġ ������ 1�� �Ǿ�� �Ѵ�.

	void AddBoneData(int boneIndex, float weight)
	{
		// ��� �ϳ��� �����Ͱ� ����־�� �Ѵ�.
		// 4�� �̻��� ���� ����� �־��� ���� �ڵ�.
		assert(BlendWeights[0] == 0.0f || BlendWeights[1] == 0.0f ||
			BlendWeights[2] == 0.0f || BlendWeights[3] == 0.0f);

		for (int i = 0; i < 4; i++)
		{
			if (BlendWeights[i] == 0.0f)
			{
				BlendIndeces[i] = boneIndex;
				BlendWeights[i] = weight;
				return;
			}
		}
	}
};

struct BoneReference
{
	std::string NodeName; // ��� �̸�
	Matrix OffsetMatrix;  // ������ �޽��� OffsetMatrix
	Matrix* NodeWorldMatrixPtr;
	int BoneIndex = -1;			 // �� �ε���
}; 

struct aiMesh;

class Mesh
{
public:
	Mesh();
	~Mesh();

public:
	void Create(aiMesh* mesh);
	void CreateBoneWeight(aiMesh* mesh, SkeletonResource* skeleton);
	void CreateIndexBuffer();

	template<typename T>
	void CreateVertexBuffer(T* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer);

 //	void UpdateMatrixPalette(Matrix* MatrixPalettePrt);

	std::vector<BoneWeightVertex>	m_BoneWeightVertices;
	std::vector<Vertex>				m_Vertices;
	std::vector<UINT>				m_Indices;

	ComPtr<ID3D11Buffer> m_pBWVertexBuffer;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;

	vector<BoneReference> m_BoneReferences;
	Matrix* m_pNodeWorld;

	UINT m_VertexCount = 0;
	UINT m_VertexBufferStride = 0;		// ���ؽ� �ϳ��� ũ��.
	UINT m_VertexBufferOffset = 0;		// ���ؽ� ������ ������.
	UINT m_IndexCount = 0;				// �ε��� ����.
	UINT m_MaterialIndex = 0;			// ���׸��� �ε���.

	string m_MeshName;
};

template<typename T>
inline void Mesh::CreateVertexBuffer(T* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer)
{
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(T) * vertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T(D3DRenderManager::m_pDevice->CreateBuffer(&bd, &vbData, vertexBuffer));

	// ���ؽ� ���� ����
	m_VertexCount = vertexCount;
	m_VertexBufferStride = sizeof(T);
	m_VertexBufferOffset = 0;
}
