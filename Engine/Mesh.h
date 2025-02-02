#pragma once

#include <string>
#include <vector>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <directxtk/SimpleMath.h>
#include <directxtk/WICTextureLoader.h>
#include <wrl/client.h>

#include "CommonApp.h"

using namespace DirectX;

struct Vertex
{
	FLOAT X, Y, Z;
	XMFLOAT2 UV;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	INT BlendIndices[4] = {};	// �����ϴ� ���� �ε���
	float BlendWeights[4] = { 0.f, 0.f, 0.f, 0.f };	// �����ϴ� ���� ���� ����ġ

	void AddBoneData(int boneIndex, float weight)
	{
		// �����Ͱ� �ϳ��� ����־�� �Ѵ�
		assert(BlendWeights[0] == 0.f || BlendWeights[1] == 0.f || BlendWeights[2] == 0.f || BlendWeights[3] == 0.f);
		for (UINT i = 0; i < 4; ++i)
		{
			if (BlendWeights[i] == 0.f)
			{
				BlendIndices[i] = boneIndex;
				BlendWeights[i] = weight;
				return;
			}
		}
	}
};

struct Texture
{
	std::string Type;
	std::string Path;
	ComPtr<ID3D11ShaderResourceView> Source;
};

struct TextureMapConstantBuffer
{
	BOOL UseDiffuse = false;
	BOOL UseNormal = false;
	BOOL UseSpecular = false;
	BOOL UseEmissive = false;
	BOOL UseOpacity = false;
	BOOL UseMetalness = false;
	BOOL UseRoughness = false;

	BOOL garbage;

	SimpleMath::Vector4 BaseColor = { 1.f, 1.f, 1.f, 1.f };
};

class Mesh
{
private:
	std::vector<Vertex> m_vertices;
	std::vector<UINT> m_indices;
	std::vector<Texture> m_textures;
	SimpleMath::Vector4 m_baseColor;

	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;

	ComPtr<ID3D11Buffer> m_pTextureMapConstantBuffer;

public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT>& indices, const std::vector<Texture>& textures, SimpleMath::Vector4 baseColor);
	~Mesh();

public:
	void Render();

private:
	void SetupMesh();
};