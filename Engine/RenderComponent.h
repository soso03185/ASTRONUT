#pragma once
#include "SceneComponent.h"

/// <summary>
/// �������� �ʿ��� ������Ʈ�� �ش� Ŭ������ ��ӹ޾� �����Ѵ�.
///	AABB�� ���� ī�޶� �ø��ϴ� ���� �ֵ� ��ǥ
/// �ν��Ͻ�ȭ���� �ʴ´�.
/// </summary>
class RenderComponent
	: public SceneComponent
{
public:
	RenderComponent() = default;
	~RenderComponent() override = default;

public:
	Math::Vector3 GetBoundingBoxOffset() const { return m_boundingBoxOffset; }
	void SetBoundingBoxOffset(Math::Vector3 offset) { m_boundingBoxOffset = offset; }
	void SetVisible(bool visible) { m_bVisible = visible; }

public:
	void Update(float deltaTime) override;


protected:
	int m_renderOrder = 0;
	bool m_bCulled = true;
	bool m_bVisible = true;

	DirectX::BoundingBox m_boundingBox;
	Math::Vector3 m_boundingBoxOffset = Math::Vector3::Zero;
};

