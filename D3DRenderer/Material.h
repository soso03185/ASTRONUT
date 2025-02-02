#pragma once

struct aiMaterial;


enum MaterialMapFlags
{
	BASECOLOR = 1 << 0,
	NORMAL = 1 << 1,
	SPECULAR = 1 << 2,
	EMISSIVE = 1 << 3,
	OPACITY = 1 << 4,
	METALNESS = 1 << 5,
	ROUGHNESS = 1 << 6
};


class MaterialTexture
{
public:
	MaterialTexture();
	~MaterialTexture();

	std::wstring m_FilePath;
	ComPtr<ID3D11ShaderResourceView> m_pTextureSRV = nullptr;	// �ؽ�ó ���ҽ� ��.
	void Create(const std::wstring& filePath);
};


class Material
{
public:
	Material();
	~Material();

	void Create(aiMaterial* pMaterial);
	void SetTexture(MaterialMapFlags materialFlag, std::wstring texturePath);

	Vector4 m_Color;

	std::shared_ptr<MaterialTexture> m_pDiffuseRV = nullptr;	// �ؽ�ó ���ҽ� ��.
	std::shared_ptr<MaterialTexture> m_pNormalRV = nullptr;	    // �븻�� ���ҽ� ��.
	std::shared_ptr<MaterialTexture> m_pSpecularRV = nullptr;	// ����ŧ���� ���ҽ� ��.
	std::shared_ptr<MaterialTexture> m_pEmissiveRV = nullptr;	// �̹̽ú�� ���ҽ� ��.
	std::shared_ptr<MaterialTexture> m_pOpacityRV = nullptr;	// ����� ���ҽ� ��.	
	std::shared_ptr<MaterialTexture> m_pMetalnessRV = nullptr;	// ��Ż�� ���ҽ� ��.	
	std::shared_ptr<MaterialTexture> m_pRoughnessRV = nullptr;	// �����Ͻ��� ���ҽ� ��.	

	UINT m_MaterialMapFlags = 0;
};
	