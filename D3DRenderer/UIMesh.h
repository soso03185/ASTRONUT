#pragma once

struct UIVertex
{
	Vector4 Position;
	Vector2 TexCoord;
};

class UIMesh
{
public:
	UIMesh();
	~UIMesh();

public:
	ComPtr<ID3D11Buffer> GetVertexBuffer() { return m_pVertexBuffer; }
	ComPtr<ID3D11Buffer> GetIndexBuffer() { return m_pIndexBuffer; }
	UINT* GetVertexBufferStride() { return &m_VertexBufferStride; }
	UINT* GetVertexBufferOffset() { return &m_VertexBufferOffset; }
	UINT GetIndexCount() { return m_IndexCount; }

public:
	void Create(Math::Vector2 size);
	void CreateIndexBuffer();
	void CreateVertexBuffer();

private:
	vector<UIVertex> m_UIVertices;
	vector<UINT> m_Indices;

	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;

	UINT m_VertexCount = 0;
	UINT m_VertexBufferStride = 0;		// ���ؽ� �ϳ��� ũ��.
	UINT m_VertexBufferOffset = 0;		// ���ؽ� ������ ������.
	UINT m_IndexCount = 0;				// �ε��� ����.
};

