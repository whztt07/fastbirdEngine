#pragma once

class btRigidBody;
class btDiscreteDynamicsWorld;
namespace fastbird
{
	class IPhysicsInterface;
	struct RotationInfo;

	
	class CLASS_DECLSPEC_PHYSICS RigidBody
	{
	public:
		RigidBody() {}
		virtual ~RigidBody(){}

		// Physics dll internal
		virtual void RefreshColShape(IPhysicsInterface* colProvider) = 0;
		virtual void ApplyForce(const Vec3& force, const Vec3& rel_pos) = 0;
		virtual void ApplyImpulse(const Vec3& impulse, const Vec3& rel_pos) = 0;
		virtual void ApplyCentralImpulse(const Vec3& impulse) = 0;
		virtual void ApplyTorqueImpulse(const Vec3& torque) = 0;
		virtual void ApplyTorque(const Vec3& torque) = 0;
		virtual void ClearForces() = 0;
		virtual float GetSpeed() const = 0;
		virtual Vec3 GetVelocity() const = 0;
		virtual Vec3 GetAngularVelocity() const = 0;
		virtual Vec3 GetTorque() const = 0;
		virtual void SetVelocity(const Vec3& vel) = 0;
		virtual void Activate() = 0;
		virtual void EnableDeactivation(bool enable) = 0;

		virtual Vec3 GetDestDir() const = 0;

		virtual void Update(float dt) = 0;

		virtual void SetMass(float mass) = 0;

		virtual void SetPhysicsInterface(IPhysicsInterface* obj) = 0;
		virtual IPhysicsInterface* GetPhysicsInterface() const = 0;

		// idx is for compound shape
		virtual void* GetColShapeUserPtr(int idx = 0) = 0;
		// rigid body user ptr
		virtual void* GetGamePtr() const = 0;

		virtual void SetRotationalForce(float force) = 0;

		virtual void RemoveColFlag(unsigned flag)=0;
		virtual void AddColFlag(unsigned flag)=0;
		virtual void SetColMask(unsigned mask) = 0;
		virtual unsigned GetColMask() const = 0;

		virtual void SetLinearDamping(float damping) = 0;
		virtual bool HasContact() = 0;
	};
}