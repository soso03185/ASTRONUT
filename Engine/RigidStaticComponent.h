#pragma once
#include "PxPhysicsAPI.h"
#include "RigidBodyComponent.h"

using namespace physx;

/// <summary>
/// RigidStaticComponent
/// ���� �ִ� ���� ��ü�� ���� ����ƽ ������ �ٵ� ������Ʈ
/// CreateStaticRigidBody�� ������ �ٵ� ������. ���� ���� ����� �ּ� ����
/// CreateStaticRigidBody �� �� isTrigger true �ϸ� ������ٵ� ��ü�� Ʈ���ŷ� �����ǰ�,
/// AddTriggerShape �Լ��� �߰� Ʈ���� ������ ���� ���� �� �ִ�.
/// ��� ������ �� CreatePlane �Լ� ���
/// </summary>

class StaticMeshComponent;

class RigidStaticComponent
	: public RigidBodyComponent, public std::enable_shared_from_this<RigidStaticComponent>
{
public:
	virtual ~RigidStaticComponent();

	PxRigidStatic* GetRigidStatic() { return m_rigidStatic; }

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
	PxRigidStatic* CreateStaticRigidBody(Geometry geometryType, const std::vector<float>& geometryArgs, const std::vector<float>& materialArgs, const Math::Vector3& offsetTrasnform = { 0.f, 0.f, 0.f }, const Math::Vector3& offsetRotation = { 0.f, 0.f, 0.f }, bool isTrigger = false);

	PxRigidStatic* CreateStaticRigidBodyFromStaticMesh(const StaticMeshComponent* staticMeshComponent, const std::vector<float>& materialArgs);

	void AddTriggerShape(Geometry geometryType, const std::vector<float>& geometryArgs, const Math::Vector3& offsetTrasnform = { 0.f, 0.f, 0.f }, const Math::Vector3& offsetRotation = { 0.f, 0.f, 0.f });

	PxRigidStatic* CreatePlane(float nx, float ny, float nz, float distance, const std::vector<float>& materialArgs);

	PxRigidStatic* CreateHollowBox(float Width, float Height, float Depth, const std::vector<float>& materialArgs);

	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;

private:
	PxRigidStatic* m_rigidStatic;
};

template<typename CollisionHandlerType>
	requires std::is_base_of<CollisionHandler, CollisionHandlerType>::value
inline void RigidStaticComponent::AddCollisionHandler()
{
	if (!m_collisionHandler) {
		m_collisionHandler = PhysicsManager::GetInstance()->GetCollisionHandler(m_rigidStatic).lock();
		if (!m_collisionHandler) {
			m_collisionHandler = std::make_shared<CollisionHandlerType>();
			PhysicsManager::GetInstance()->AddCollisionHandler(m_rigidStatic, m_collisionHandler);
		}
	}
}