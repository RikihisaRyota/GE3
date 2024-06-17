#include "Collider.h"

#include <array>

#include "Engine/Math/MyMath.h"
#include "Engine/DrawLine/DrawLine.h"
#include "CollisionManager.h"

namespace {
	std::vector<Vector3> GetVertices(const OBB& obb) {
		Vector3 halfSize = obb.size * 0.5f;

		std::vector<Vector3> vertices(8);
		vertices[0] = { -halfSize.x, -halfSize.y, -halfSize.z };
		vertices[1] = { -halfSize.x,  halfSize.y, -halfSize.z };
		vertices[2] = { halfSize.x,  halfSize.y, -halfSize.z };
		vertices[3] = { halfSize.x, -halfSize.y, -halfSize.z };
		vertices[4] = { -halfSize.x, -halfSize.y,  halfSize.z };
		vertices[5] = { -halfSize.x,  halfSize.y,  halfSize.z };
		vertices[6] = { halfSize.x,  halfSize.y,  halfSize.z };
		vertices[7] = { halfSize.x, -halfSize.y,  halfSize.z };

		Matrix4x4 obbWorldMatrix =
			Matrix4x4().SetXAxis(obb.orientations[0]).SetYAxis(obb.orientations[1]).SetZAxis(obb.orientations[2]).SetTranslate(obb.center);
		for (size_t i = 0; i < vertices.size(); ++i) {
			vertices[i] = vertices[i] * obbWorldMatrix;
		}

		return vertices;
	}

	Vector2 Projection(const std::vector<Vector3>& vertices, const Vector3& axis) {
		Vector2 minmax(Dot(axis, vertices[0]));
		for (size_t i = 1; i < vertices.size(); ++i) {
			float dot = Dot(axis, vertices[i]);
			minmax.x = (std::min)(dot, minmax.x);
			minmax.y = (std::max)(dot, minmax.y);
		}
		return minmax;
	}

	float GetOverlap(const Vector2& minmax1, const Vector2& minmax2) {
		float range1 = minmax1.y - minmax1.x;
		float range2 = minmax2.y - minmax2.x;
		float maxOverlap = (std::max)(minmax1.y, minmax2.y) - (std::min)(minmax1.x, minmax2.x);
		return range1 + range2 - maxOverlap;
	}
}
Collider::Collider() {
	CollisionManager::GetInstance()->AddCollider(std::move(this));
}

void Collider::OnCollision(const ColliderDesc& colliderDesc) {
	if (callback_) {
		callback_(colliderDesc);
	}
}

bool Collider::CanCollision(Collider* other) const {
	return (this->collisionAttribute_ & other->collisionMask_) && (other->collisionAttribute_ & this->collisionMask_);
}

bool Collider::CanCollision(uint32_t mask) const {
	return (this->collisionAttribute_ & mask);
}

bool OBBCollider::IsCollision(Collider* other, ColliderDesc& desc) {
	if (CanCollision(other)) {
		return  other->IsCollision(this, desc);
	}
	return false;
}

bool OBBCollider::IsCollision(OBBCollider* other, ColliderDesc& desc) {
	auto vertices1 = GetVertices(this->obb_);
	auto vertices2 = GetVertices(other->obb_);

	Vector3 axes[] = {
		this->obb_.orientations[0],
		this->obb_.orientations[1],
		this->obb_.orientations[2],

		other->obb_.orientations[0],
		other->obb_.orientations[1],
		other->obb_.orientations[2],

		Cross(this->obb_.orientations[0], other->obb_.orientations[0]).Normalized(),
		Cross(this->obb_.orientations[0], other->obb_.orientations[1]).Normalized(),
		Cross(this->obb_.orientations[0], other->obb_.orientations[2]).Normalized(),

		Cross(this->obb_.orientations[1], other->obb_.orientations[0]).Normalized(),
		Cross(this->obb_.orientations[1], other->obb_.orientations[1]).Normalized(),
		Cross(this->obb_.orientations[1], other->obb_.orientations[2]).Normalized(),

		Cross(this->obb_.orientations[2], other->obb_.orientations[0]).Normalized(),
		Cross(this->obb_.orientations[2], other->obb_.orientations[1]).Normalized(),
		Cross(this->obb_.orientations[2], other->obb_.orientations[2]).Normalized(),
	};

	float minOverlap = FLT_MAX;
	Vector3 minOverlapAxis = {};

	auto IsSeparateAxis = [&](const Vector3& axis) {
		if (axis.LengthSquared() < 1e-6) { return false; }
		if (axis == Vector3(0.0f,0.0f,0.0f)) { return false; }
		Vector2 minmax1 = Projection(vertices1, axis);
		Vector2 minmax2 = Projection(vertices2, axis);

		if (!(minmax1.x <= minmax2.y && minmax1.y >= minmax2.x)) {
			return true;
		}

		float overlap = GetOverlap(minmax1, minmax2);
		if (overlap < minOverlap) {
			minOverlapAxis = axis;
			minOverlap = overlap;
		}

		return false;
		};

	for (auto& axis : axes) {
		if (std::isnan(axis.x) || std::isnan(axis.y) || std::isnan(axis.z)) { continue; }
		if (IsSeparateAxis(axis)) { return false; }
	}

	desc.collider = this;
	desc.normal = minOverlapAxis.Normalized();
	if (Dot(other->obb_.center - this->obb_.center, desc.normal) < 0.0f) {
		desc.normal *= -1.0f;
	}
	desc.depth = minOverlap;
	return true;
}

void OBBCollider::DrawCollision(const ViewProjection& viewProjection, const Vector4& color) {
	auto drawLine = DrawLine::GetInstance();
	std::vector<Vector3> vertices = GetVertices(obb_);
	const std::vector<std::pair<int, int>> edges = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, // 前面の4つの辺
		{4, 5}, {5, 6}, {6, 7}, {7, 4}, // 背面の4つの辺
		{0, 4}, {1, 5}, {2, 6}, {3, 7}  // 前面と背面を結ぶ4つの辺
	};

	// Draw the edges
	for (const auto& edge : edges) {
		drawLine->SetLine(vertices[edge.first], vertices[edge.second], color);
	}

}
