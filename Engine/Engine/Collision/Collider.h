#pragma once

#include <functional>
#include <string>

#include "Engine/Math/OBB.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

class Collider;
class OBBCollider;
struct ViewProjection;

struct ColliderDesc {
	Collider* collider;
	Vector3 normal;
	float depth;
};

class Collider {
public:
	using Callback = std::function<void(const ColliderDesc&)>;

	Collider();

	virtual bool IsCollision(Collider* collider, ColliderDesc& desc) = 0;
	virtual bool IsCollision(OBBCollider* collider, ColliderDesc& desc) = 0;
	virtual void DrawCollision(const ViewProjection& viewProjection, const Vector4& color) = 0;

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

class OBBCollider :
	public Collider {
public:
	bool IsCollision(Collider* other, ColliderDesc& desc) override;
	bool IsCollision(OBBCollider* other, ColliderDesc& desc) override;
	void DrawCollision(const ViewProjection& viewProjection, const Vector4& color) override;

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