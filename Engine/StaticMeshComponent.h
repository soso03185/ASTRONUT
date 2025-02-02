#pragma once
#include "RenderComponent.h"

class StaticMeshModel;

/// <summary>
///	StaticMesh�� �������ϱ� ���� ������Ʈ
///	Renderer�� StaticMeshModel�� ����� �����Ѵ�.
/// </summary>
class StaticMeshComponent
	: public RenderComponent
{
public:
	StaticMeshComponent() = default;
	~StaticMeshComponent() override;

public:
	std::shared_ptr<StaticMeshModel> GetStaticMeshModel() const { return m_staticMeshModel; }

	// CreateComponent �� �ʼ������� filePath ����
	void SetFilePath(std::string_view filePath) { m_filePath = filePath; }
	void SetOutLine(bool isOutLine);

	Math::Vector3 GetModelDimensions() const;

public:
	void Initialize() override;	// staticMeshModel�� �������� ���� ����, �޾ƿ´�.
	void Update(float deltaTime) override;


private:
	std::string m_filePath;
	std::shared_ptr<StaticMeshModel> m_staticMeshModel;
};

