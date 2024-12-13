#pragma once
/**
 * @file Collider.h
 * @brief Collider
 */
#include <functional>
#include <string>

#include "Engine/Math/Capsule.h"
#include "Engine/Math/Sphere.h"
#include "Engine/Math/OBB.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

class Collider;
class OBBCollider;
class SphereCollider;
class CapsuleCollider;

struct ColliderDesc {
	Collider* collider;
	Vector3 normal;
	float depth;
};

class Collider {
public:
	using Callback = std::function<void(const ColliderDesc&)>;

	Collider();
	~Collider();

	virtual bool IsCollision(Collider* collider, ColliderDesc& desc) = 0;
	virtual bool IsCollision(OBBCollider* collider, ColliderDesc& desc) = 0;
	virtual bool IsCollision(SphereCollider* collider, ColliderDesc& desc) = 0;
	virtual bool IsCollision(CapsuleCollider* collider, ColliderDesc& desc) = 0;
	virtual void DrawCollision(const Vector4& color) = 0;
	
	virtual const Vector3& GetPosition() = 0;

	void SetCallback(Callback callback) { callback_ = callback; }
	void SetCollisionAttribute(uint32_t attribute) { collisionAttribute_ = attribute; }
	void SetCollisionMask(uint32_t mask) { collisionMask_ = mask; }
	void SetIsActive(bool isActive) { isActive_ = isActive; }
	void SetName(const std::string& name) { name_ = name; }
	const std::string& GetName()const { return name_; }

	void OnCollision(const ColliderDesc& colliderDesc);

	const bool GetIsActive()const { return isActive_; }
protected:
	bool CanCollision(Collider* other) const;
	bool CanCollision(uint32_t mask) const;

	std::string name_;
	Callback callback_;
	uint32_t collisionAttribute_ = 0xFFFFFFFF;
	uint32_t collisionMask_ = 0xFFFFFFFF;
	bool isActive_ = true;
};

class SphereCollider :
	public Collider {
	friend class OBBCollider;
	friend class CapsuleCollider;
public:
	bool IsCollision(Collider* collider, ColliderDesc& collisionInfo) override;
	bool IsCollision(SphereCollider* collider, ColliderDesc& collisionInfo) override;
	bool IsCollision(OBBCollider* collider, ColliderDesc& collisionInfo) override;
	bool IsCollision(CapsuleCollider* other, ColliderDesc& desc) override;
	void DrawCollision(const Vector4& color) override;

	const Vector3& GetPosition() override;

	void SetCenter(const Vector3& center) { sphere_.center = center; }
	void SetRadius(float radius) { sphere_.radius = radius; }
	void SetSphere(const Sphere& sphere) { sphere_ = sphere; }

	Vector3& GetCenter() { return sphere_.center; }
	float& GetRadius() { return sphere_.radius; }
	Sphere& GetSphere() { return  sphere_; }
private:
	Sphere sphere_;
};

class OBBCollider :
	public Collider {
	friend class SphereCollider;
	friend class CapsuleCollider;
public:
	bool IsCollision(Collider* other, ColliderDesc& desc) override;
	bool IsCollision(SphereCollider* collider, ColliderDesc& collisionInfo) override;
	bool IsCollision(OBBCollider* other, ColliderDesc& desc) override;
	bool IsCollision(CapsuleCollider* other, ColliderDesc& desc) override;
	void DrawCollision(const Vector4& color) override;

	const Vector3& GetPosition() override;

	void SetCenter(const Vector3& center) { obb_.center = center; }
	void SetOrientation(const Quaternion& orientation) {
		obb_.orientations[0] = orientation.GetRight();
		obb_.orientations[1] = orientation.GetUp();
		obb_.orientations[2] = orientation.GetForward();
	}
	void SetSize(const Vector3& size) { obb_.size = size; }

private:
	OBB obb_;
};

class CapsuleCollider :
	public Collider {
	friend class SphereCollider;
	friend class OBBCollider;
public:
	bool IsCollision(Collider* other, ColliderDesc& desc) override;
	bool IsCollision(SphereCollider* collider, ColliderDesc& collisionInfo) override;
	bool IsCollision(OBBCollider* other, ColliderDesc& desc) override;
	bool IsCollision(CapsuleCollider* other, ColliderDesc& desc) override;
	void DrawCollision(const Vector4& color) override;

	const Vector3& GetPosition() override;

	void SetSegment(const Segment& segment) { capsule_.segment = segment; }
	void SetRadius(float radius) { capsule_.radius = radius; }
	void SetCapsule(const Capsule& capsule) { capsule_ = capsule; }
private:
	Capsule capsule_;
	Vector3 center_;
};