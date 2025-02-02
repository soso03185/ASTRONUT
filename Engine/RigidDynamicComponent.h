#pragma once
#include "PxPhysicsAPI.h"
#include "RigidBodyComponent.h"

using namespace physx;

/// <summary>
/// RigidDynamicComponent
/// �����̴� ���� ��ü�� ���� ���̳��� ������ �ٵ� ������Ʈ
/// CreateDynamicRigidBody�� ������ �ٵ� ������. ���� ���� ����� �ּ� ����
/// CreateDynamicRigidBody �� �� isTrigger true �ϸ� ������ٵ� ��ü�� Ʈ���ŷ� �����ǰ�,
/// AddTriggerShape �Լ��� �߰� Ʈ���� ������ ���� ���� �� �ִ�.
/// CreateDynamicRigidBody ���� �߿� density �е��� ���� �����ϴ°ǵ�
/// SetMass�Լ��� ������ ������ ���� �ִ�. (�׽�Ʈ �� �غ�)
/// SetLockFlags�� �̵��̳� ȸ���� �� �� �� ����. ���ڴ� PxRigidDynamicLockFlags ����
/// </summary>
class RigidDynamicComponent
	: public RigidBodyComponent, public std::enable_shared_from_this<RigidDynamicComponent>
{
public:
	virtual ~RigidDynamicComponent();
	PxRigidDynamic* GetRigidDynamic() { return m_rigidDynamic; }

	template<typename CollisionHandlerType>
		requires std::is_base_of<CollisionHandler, CollisionHandlerType>::value
	void AddCollisionHandler();
	//���� Ÿ�Ժ� ���� ���缭 {}�� ����
	//geometryArgs
	//Sphere : radian
	//Capsule : radian, halfHeight
	//Box : halfX, halfY, halfZ
	//materialArgs
	//staticFriction : ���� ������
	//dynamicFriction : � ������
	//restitution : �ݹ� ���
	//density : �е�
	PxRigidDynamic* CreateDynamicRigidBody(Geometry gesometryType, const std::vector<float>& geometryArgs, const std::vector<float>& materialArgs, float density = 1.f, const Math::Vector3& offsetTrasnform = { 0.f, 0.f, 0.f }, const Math::Vector3& offsetRotation = { 0.f, 0.f, 0.f }, bool isTrigger = false);

	void AddTriggerShape(Geometry geometryType, const std::vector<float>& geometryArgs, const Math::Vector3& offsetTrasnform = { 0.f, 0.f, 0.f }, const Math::Vector3& offsetRotation = { 0.f, 0.f, 0.f });

	void SetMass(float mass, Math::Vector3 centerOfMassLocalPos = { 0.f, 0.f, 0.f });
	void SetLockFlags(PxRigidDynamicLockFlags flags);
	void SetTransform(Math::Matrix& worldMatrix);

	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

private:
	PxRigidDynamic* m_rigidDynamic;
};

template<typename CollisionHandlerType>
	requires std::is_base_of<CollisionHandler, CollisionHandlerType>::value
inline void RigidDynamicComponent::AddCollisionHandler()
{
	if (!m_collisionHandler) {
		m_collisionHandler = PhysicsManager::GetInstance()->GetCollisionHandler(m_rigidDynamic).lock();
		if (!m_collisionHandler) {
			m_collisionHandler = std::make_shared<CollisionHandlerType>();
			PhysicsManager::GetInstance()->AddCollisionHandler(m_rigidDynamic, m_collisionHandler);
		}
	}
}