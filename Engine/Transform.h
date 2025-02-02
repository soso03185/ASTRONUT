#pragma once

#include <directxtk/SimpleMath.h>
namespace Math = DirectX::SimpleMath;

class GameObject;
/// <summary>
/// �θ��� ��ġ�� �ݿ��� ��ġ ������Ʈ�� ���� ������Ʈ
///	�ͽ������� ��ӹ޾� ����Ѵ�
/// </summary>
class Transform
{
public:
	Transform();
	~Transform() = default;

	const Math::Vector3 GetWorldPosition() const;

	void SetParent(Transform* pParent) { m_pParent = pParent; }

	virtual void Update(float deltaTime);

	Math::Matrix m_localMatrix;
	Math::Matrix m_worldMatrix;

	GameObject* m_owner;
	Transform* m_pParent = nullptr;
};

