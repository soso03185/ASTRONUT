#pragma once
#include "RenderComponent.h"

class EnvironmentModel;
/// <summary>
///	Environment Cube�� �������ϱ� ���� ������Ʈ
///	Renderer�� EnvironmentModel�� ����� �����Ѵ�.
/// </summary>
class EnvironmentComponent
	: public RenderComponent
{
	// transform ������Ʈ �ű��
	// ���� �̸� ���� �ű��
	// renderer�� m_pEnvironment�� set ���ֱ�

public:
	EnvironmentComponent() = default;
	~EnvironmentComponent() override;

public:
	std::shared_ptr<EnvironmentModel> GetEnvironmentModel() const { return m_environmentModel; }

	// CreateComponent �� �ʼ������� ������ filePath��
	// fbx file
	void SetSceneFilePath(std::string_view filePath) { m_filePath = filePath; }
	// dds file
	void SetEnvironmentTexturePath(std::wstring_view filePath) { m_environmentTexturePath = filePath; }
	void SetIBLSpecularTexturePath(std::wstring_view filePath) { m_IBLSpecularTexturePath = filePath; }
	void SetIBLDiffuseTexturePath(std::wstring_view filePath) { m_IBLDiffuseTexturePath = filePath; }
	void SetIBLBRDFTexturePath(std::wstring_view filePath) { m_IBLBRDFTexturePath = filePath; }

public:
	void Initialize() override;	// staticMeshModel�� �������� ���� ����, �޾ƿ´�.
	void Update(float deltaTime) override;


private:
	std::string m_filePath;
	std::wstring m_environmentTexturePath;
	std::wstring m_IBLSpecularTexturePath;
	std::wstring m_IBLDiffuseTexturePath;
	std::wstring m_IBLBRDFTexturePath;

	std::shared_ptr<EnvironmentModel> m_environmentModel;
};