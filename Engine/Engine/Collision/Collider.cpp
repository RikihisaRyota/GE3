#include "Collider.h"

#include <array>
#include <numbers>

#include "Engine/Math/MyMath.h"
#include "Engine/DrawLine/DrawLine.h"
#include "CollisionManager.h"

namespace {
	std::vector<Vector3> GetVertices(const OBB& obb) {
		Vector3 halfSize = obb.size * 0.5f;

		std::vector<Vector3> vertices(8);
		vertices[0] = obb.center + (obb.orientations[0] * -halfSize.x) + (obb.orientations[1] * -halfSize.y) + (obb.orientations[2] * -halfSize.z);
		vertices[1] = obb.center + (obb.orientations[0] * -halfSize.x) + (obb.orientations[1] * halfSize.y) + (obb.orientations[2] * -halfSize.z);
		vertices[2] = obb.center + (obb.orientations[0] * halfSize.x) + (obb.orientations[1] * halfSize.y) + (obb.orientations[2] * -halfSize.z);
		vertices[3] = obb.center + (obb.orientations[0] * halfSize.x) + (obb.orientations[1] * -halfSize.y) + (obb.orientations[2] * -halfSize.z);
		vertices[4] = obb.center + (obb.orientations[0] * -halfSize.x) + (obb.orientations[1] * -halfSize.y) + (obb.orientations[2] * halfSize.z);
		vertices[5] = obb.center + (obb.orientations[0] * -halfSize.x) + (obb.orientations[1] * halfSize.y) + (obb.orientations[2] * halfSize.z);
		vertices[6] = obb.center + (obb.orientations[0] * halfSize.x) + (obb.orientations[1] * halfSize.y) + (obb.orientations[2] * halfSize.z);
		vertices[7] = obb.center + (obb.orientations[0] * halfSize.x) + (obb.orientations[1] * -halfSize.y) + (obb.orientations[2] * halfSize.z);

		return vertices;
	}

	Vector2 Projection(const std::vector<Vector3>& vertices, const Vector3& axis) {
		float min = Dot(axis, vertices[0]);
		float max = min;
		for (size_t i = 1; i < vertices.size(); ++i) {
			float dot = Dot(axis, vertices[i]);
			if (dot < min) min = dot;
			if (dot > max) max = dot;
		}
		return Vector2(min, max);
	}

	float GetOverlap(const Vector2& minmax1, const Vector2& minmax2) {
		return (std::min)(minmax1.y, minmax2.y) - (std::max)(minmax1.x, minmax2.x);
	}

	std::vector<Vector3> GenerateCircleVertices(const Vector3& center, float radius, int segments, const Vector3& axis1, const Vector3& axis2) {
		std::vector<Vector3> vertices;
		vertices.resize(segments);
		float angleStep = 2.0f * std::numbers::pi_v<float> / segments;
		for (int i = 0; i < segments; ++i) {
			float angle = i * angleStep;
			Vector3 vertex = center + (axis1 * cos(angle) + axis2 * sin(angle)) * radius;
			vertices[i] = vertex; // Assign the computed vertex directly
		}
		return vertices;
	}

	Vector3 ClosestPointOnSegment(const Segment& segment, const Vector3& point) {
		Vector3 segmentDir = segment.end - segment.start;
		float t = Dot(point - segment.start, segmentDir) / Dot(segmentDir, segmentDir);
		t = std::clamp(t, 0.0f, 1.0f);
		return segment.start + segmentDir * t;
	}


	float ClosestDistanceBetweenSegments(const Segment& seg1, const Segment& seg2, Vector3& closestPoint1, Vector3& closestPoint2) {
		const Vector3& p1 = seg1.start;
		const Vector3& q1 = seg1.end;
		const Vector3& p2 = seg2.start;
		const Vector3& q2 = seg2.end;

		Vector3 d1 = q1 - p1;  // Direction vector of segment S1
		Vector3 d2 = q2 - p2;  // Direction vector of segment S2
		Vector3 r = p1 - p2;

		float a = Dot(d1, d1);  // Squared length of segment S1, always nonnegative
		float e = Dot(d2, d2);  // Squared length of segment S2, always nonnegative
		float f = Dot(d2, r);

		float s, t;
		float b = Dot(d1, d2);
		float c = Dot(d1, r);
		float denom = a * e - b * b;  // Always nonnegative

		// If segments are not parallel, compute the closest points
		if (denom != 0.0f) {
			s = (b * f - c * e) / denom;
			t = (a * f - b * c) / denom;

			// Clamp s to [0, 1]
			if (s < 0.0f) s = 0.0f;
			else if (s > 1.0f) s = 1.0f;

			// Clamp t to [0, 1]
			if (t < 0.0f) t = 0.0f;
			else if (t > 1.0f) t = 1.0f;
		}
		else {
			// If segments are parallel, we can choose any t in [0, 1] and solve for s
			s = 0.0f;  // Use the start of the first segment
			t = f / e;

			// Clamp t to [0, 1]
			if (t < 0.0f) t = 0.0f;
			else if (t > 1.0f) t = 1.0f;
		}

		closestPoint1 = p1 + d1 * s;
		closestPoint2 = p2 + d2 * t;

		return (closestPoint1 - closestPoint2).LengthSquared();
	}

	Vector3 ClosestPointOnOBB(const OBB& obb, const Vector3& point) {
		Vector3 d = point - obb.center;
		Vector3 closestPoint = obb.center;
		Vector3 obbHalfSize = obb.size * 0.5f;

		// Project point onto OBB axes and clamp to box extents
		for (int i = 0; i < 3; ++i) {
			float dist = Dot(d, obb.orientations[i]);
			if (i == 0) {
				if (dist > obbHalfSize.x) {
					dist = obbHalfSize.x;
				}
				if (dist < -obbHalfSize.x) {
					dist = -obbHalfSize.x;
				}
			}
			else if (i == 1) {
				if (dist > obbHalfSize.y) {
					dist = obbHalfSize.y;
				}
				if (dist < -obbHalfSize.y) {
					dist = -obbHalfSize.y;
				}
			}
			else if (i == 2) {
				if (dist > obbHalfSize.z) {
					dist = obbHalfSize.z;

				}
				if (dist < -obbHalfSize.z) {
					dist = -obbHalfSize.z;
				}
			}
			closestPoint += obb.orientations[i] * dist;
		}

		return closestPoint;
	}

	std::vector<Vector3> GenerateHalfSphereVertices(const Vector3& center, float radius, int segments, const Vector3& axis1, const Vector3& axis2) {
		std::vector<Vector3> vertices;
		vertices.reserve(segments + 1); // Reserve space for the vertices, including the center
		float angleStep = std::numbers::pi_v<float> / segments;
		for (int i = 0; i <= segments; ++i) {
			float angle = i * angleStep;
			Vector3 vertex = center + (axis1 * cos(angle) + axis2 * sin(angle)) * radius;
			vertices.push_back(vertex);
		}

		return vertices;
	}


	std::vector<Vector3> GenerateHalfSphereVertices(const Vector3& center, float radius, int rings, int sectors, const Vector3& axis1, const Vector3& axis2) {
		std::vector<Vector3> vertices;
		vertices.reserve((rings + 1) * (sectors + 1)); // Reserve space for the vertices, including the center

		float ringStep = std::numbers::pi_v<float> / rings;
		float sectorStep = 2.0f * std::numbers::pi_v<float> / sectors;

		for (int r = 0; r <= rings; ++r) {
			float ringAngle = r * ringStep;
			float sinRing = sin(ringAngle);
			float cosRing = cos(ringAngle);

			for (int s = 0; s <= sectors; ++s) {
				float sectorAngle = s * sectorStep;
				Vector3 vertex = center + (axis1 * sinRing * cos(sectorAngle) + axis2 * sinRing * sin(sectorAngle)) * radius;
				vertices.push_back(vertex);
			}
		}

		return vertices;
	}

}
Collider::Collider() {
	CollisionManager::GetInstance()->AddCollider(this);
}

Collider::~Collider() {
	CollisionManager::GetInstance()->DeleteCollider(this);
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

bool OBBCollider::IsCollision(SphereCollider* collider, ColliderDesc& collisionInfo) {
	Sphere& sphere = collider->sphere_;
	OBB& obb = this->obb_;
	// obbのローカル空間で衝突判定を行う
	Matrix4x4 obbRotateMatrix = Matrix4x4().SetXAxis(obb.orientations[0]).SetYAxis(obb.orientations[1]).SetZAxis(obb.orientations[2]);
	Matrix4x4 tmp = Transpose(obbRotateMatrix);
	Matrix4x4 obbWorldInverse = tmp.SetTranslate(-obb.center * tmp);
	Vector3 centerInOBBLocal = sphere.center * obbWorldInverse;
	Vector3 halfSize = obb.size * 0.5f;

	Vector3 point = {
		  std::clamp(centerInOBBLocal.x, -halfSize.x, halfSize.x),
		  std::clamp(centerInOBBLocal.y, -halfSize.y, halfSize.y),
		  std::clamp(centerInOBBLocal.z, -halfSize.z, halfSize.z) };
	Vector3 diff = centerInOBBLocal - point;

	if (diff.LengthSquared() > sphere.radius * sphere.radius) {
		return false;
	}

	// 衝突情報を格納していく
	float length = diff.Length();
	collisionInfo.collider = this;
	if (length != 0.0f) {
		collisionInfo.normal = (diff / length) * obbRotateMatrix;
	}
	else {
		collisionInfo.normal = Vector3(0.0f, 0.0f, 0.0f);
	}
	collisionInfo.depth = sphere.radius - length;
	return true;
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
		// Skip zero-length axis
		if (axis.LengthSquared() < 1e-6) { return false; }

		Vector2 minmax1 = Projection(vertices1, axis);
		Vector2 minmax2 = Projection(vertices2, axis);

		// Check for no overlap
		if (minmax1.y < minmax2.x || minmax2.y < minmax1.x) {
			return true; // Separating axis found
		}

		// Calculate overlap
		float overlap = GetOverlap(minmax1, minmax2);
		if (overlap < minOverlap) {
			minOverlapAxis = axis;
			minOverlap = overlap;
		}

		return false;
		};

	// Check all axes
	for (auto& axis : axes) {
		// Skip invalid axes
		if (std::isnan(axis.x) || std::isnan(axis.y) || std::isnan(axis.z)) { continue; }
		if (IsSeparateAxis(axis)) { return false; }
	}

	// No separating axis found, collision detected
	desc.collider = this;
	desc.normal = minOverlapAxis.Normalized();
	if (Dot(other->obb_.center - this->obb_.center, desc.normal) < 0.0f) {
		desc.normal *= -1.0f;
	}
	desc.depth = minOverlap;
	return true;
}

bool OBBCollider::IsCollision(CapsuleCollider* other, ColliderDesc& desc) {
	const Capsule& capsule = other->capsule_;
	const OBB& obb = obb_;

	Vector3 p1 = capsule.segment.start;
	Vector3 p2 = capsule.segment.end;
	Vector3 m = obb.center - p1;
	Vector3 closestSegment = ClosestPointOnSegment(capsule.segment,m);
	Vector3 localPoint = closestSegment - obb.center;

	Vector3 normal = { 0, 0, 0 };
	float minPenetration = FLT_MAX;

	for (uint32_t i = 0; i < 3; ++i) {
		float distance = localPoint.Dot(obb.orientations[i]);
		float extent = 0.0f;

		if (i == 0) {
			extent = obb.size.x * 0.5f;
		}
		else if (i == 1) {
			extent = obb.size.y * 0.5f;
		}
		else if (i == 2) {
			extent = obb.size.z * 0.5f;
		}

		float penetration = extent + capsule.radius - std::fabs(distance);
		if (penetration < 0) {
			return false;
		}

		if (penetration < minPenetration) {
			minPenetration = penetration;
			normal = obb.orientations[i] * ((distance < 0) ? -1.0f : 1.0f);
		}
	}

	desc.collider = this;
	desc.normal = normal.Normalized();
	desc.depth = minPenetration;
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

bool SphereCollider::IsCollision(Collider* collider, ColliderDesc& collisionInfo) {
	if (CanCollision(collider)) {
		return  collider->IsCollision(this, collisionInfo);
	}
	return false;
}

bool SphereCollider::IsCollision(SphereCollider* collider, ColliderDesc& collisionInfo) {
	// 差分ベクトルを求める
	Vector3 diff = collider->sphere_.center - this->sphere_.center;
	float hitRange = this->sphere_.radius + collider->sphere_.radius;
	if (diff.LengthSquared() > hitRange * hitRange) {
		return false;
	}

	// 衝突情報を格納していく
	float length = diff.Length();
	collisionInfo.collider = this;
	if (length != 0.0f) {
		collisionInfo.normal = diff / length;
	}
	else {
		collisionInfo.normal = Vector3(0.0f, 0.0f, 0.0f);
	}
	collisionInfo.depth = hitRange - length;
	return true;
}
bool SphereCollider::IsCollision(OBBCollider* collider, ColliderDesc& collisionInfo) {
	// Get the vertices of the OBB
	auto vertices = GetVertices(collider->obb_);

	// Find the closest point on the OBB to the sphere's center
	Vector3 closestPoint = ClosestPointOnOBB(collider->obb_, sphere_.center);

	// Calculate the vector from the sphere's center to the closest point on the OBB
	Vector3 diff = closestPoint - sphere_.center;
	float distSquared = diff.LengthSquared();

	// Check if the closest point is inside the sphere
	if (distSquared <= sphere_.radius * sphere_.radius) {
		// Collision detected
		float dist = sqrtf(distSquared);
		collisionInfo.collider = this;
		collisionInfo.normal = (dist > 0.0f) ? diff / dist : Vector3(0.0f, 0.0f, 0.0f);
		collisionInfo.depth = sphere_.radius - dist;
		return true;
	}

	// No collision
	return false;
}
bool SphereCollider::IsCollision(CapsuleCollider* other, ColliderDesc& desc) {
	// Calculate the closest point on the capsule segment to the sphere center
	Vector3 closestPoint = ClosestPointOnSegment(other->capsule_.segment, sphere_.center);

	// Calculate the vector from the sphere's center to the closest point on the segment
	Vector3 diff = closestPoint - sphere_.center;
	float distSquared = diff.LengthSquared();

	// Check if the closest point is inside the sphere
	if (distSquared <= sphere_.radius * sphere_.radius) {
		// Collision detected
		float dist = sqrtf(distSquared);
		desc.collider = this;
		desc.normal = (dist > 0.0f) ? diff / dist : Vector3(0.0f, 0.0f, 0.0f);
		desc.depth = sphere_.radius - dist;
		return true;
	}

	// No collision
	return false;
}

bool CapsuleCollider::IsCollision(OBBCollider* other, ColliderDesc& desc) {
	const Capsule& capsule = capsule_;
	const OBB& obb = other->obb_;

	Vector3 p1 = capsule.segment.start;
	Vector3 p2 = capsule.segment.end;
	Vector3 m = obb.center - p1;
	Vector3 closestSegment = ClosestPointOnSegment(capsule.segment, m);
	Vector3 localPoint = closestSegment - obb.center;

	Vector3 normal = { 0, 0, 0 };
	float minPenetration = FLT_MAX;

	for (uint32_t i = 0; i < 3; ++i) {
		float distance = localPoint.Dot(obb.orientations[i]);
		float extent = 0.0f;

		if (i == 0) {
			extent = obb.size.x * 0.5f;
		}
		else if (i == 1) {
			extent = obb.size.y * 0.5f;
		}
		else if (i == 2) {
			extent = obb.size.z * 0.5f;
		}

		float penetration = extent + capsule.radius - std::fabs(distance);
		if (penetration < 0) {
			return false;
		}

		if (penetration < minPenetration) {
			minPenetration = penetration;
			normal = obb.orientations[i] * ((distance < 0) ? -1.0f : 1.0f);
		}
	}

	desc.collider = this;
	desc.normal = normal.Normalized();
	desc.depth = minPenetration;
	return true;
}


void SphereCollider::DrawCollision(const ViewProjection& viewProjection, const Vector4& color) {
	auto drawLine = DrawLine::GetInstance();
	Sphere sphere = sphere_;
	const Vector3& center = sphere.center;
	float radius = sphere.radius;
	int segments = 24;

	std::vector<Vector3> xyVertices = GenerateCircleVertices(center, radius, segments, { 1, 0, 0 }, { 0, 1, 0 });
	std::vector<Vector3> xzVertices = GenerateCircleVertices(center, radius, segments, { 1, 0, 0 }, { 0, 0, 1 });
	std::vector<Vector3> yzVertices = GenerateCircleVertices(center, radius, segments, { 0, 1, 0 }, { 0, 0, 1 });

	for (int i = 0; i < segments; ++i) {
		drawLine->SetLine(xyVertices[i], xyVertices[(i + 1) % segments], color);
		drawLine->SetLine(xzVertices[i], xzVertices[(i + 1) % segments], color);
		drawLine->SetLine(yzVertices[i], yzVertices[(i + 1) % segments], color);
	}
}

bool CapsuleCollider::IsCollision(Collider* other, ColliderDesc& desc) {
	if (CanCollision(other)) {
		return  other->IsCollision(this, desc);
	}
	return false;
}

bool CapsuleCollider::IsCollision(SphereCollider* collider, ColliderDesc& collisionInfo) {
	const Sphere& sphere = collider->sphere_;
	const Segment& segment = capsule_.segment;

	// Calculate the closest point on the capsule's segment to the sphere's center
	Vector3 closestPoint = ClosestPointOnSegment(segment, sphere.center);

	// Calculate the distance from the closest point to the sphere's center
	Vector3 diff = sphere.center - closestPoint;
	float distanceSquared = diff.LengthSquared();
	float radiusSum = capsule_.radius + sphere.radius;

	if (distanceSquared > radiusSum * radiusSum) {
		return false;
	}

	// Fill collision info
	float distance = sqrt(distanceSquared);
	collisionInfo.collider = this;
	collisionInfo.normal = (distance != 0.0f) ? diff / distance : Vector3(0.0f, 0.0f, 0.0f);
	collisionInfo.depth = radiusSum - distance;
	return true;
}

bool CapsuleCollider::IsCollision(CapsuleCollider* other, ColliderDesc& desc) {
	const Capsule& capsule1 = this->capsule_;
	const Capsule& capsule2 = other->capsule_;

	Vector3 p1, p2;
	float distSquared = ClosestDistanceBetweenSegments(capsule1.segment, capsule2.segment, p1, p2);
	float radiusSum = capsule1.radius + capsule2.radius;

	if (distSquared > radiusSum * radiusSum) {
		return false;
	}

	float distance = sqrt(distSquared);
	desc.collider = this;
	desc.normal = (distance != 0.0f) ? (p2 - p1) / distance : Vector3(0.0f, 0.0f, 0.0f);
	desc.depth = radiusSum - distance;
	return true;
}

void CapsuleCollider::DrawCollision(const ViewProjection& viewProjection, const Vector4& color) {
	auto drawLine = DrawLine::GetInstance();
	const Segment& segment = capsule_.segment;
	const Vector3& p1 = segment.start;
	const Vector3& p2 = segment.end;
	float radius = capsule_.radius;
	int segments = 24;

	// Calculate the direction vector of the segment
	Vector3 direction = p2 - p1;
	direction.Normalize();

	// Find two vectors that are perpendicular to the direction vector
	Vector3 axis1, axis2;
	if (fabs(direction.x) > fabs(direction.y)) {
		axis1 = Vector3(direction.z, 0, -direction.x);
	}
	else {
		axis1 = Vector3(0, -direction.z, direction.y);
	}
	axis1.Normalize();
	axis2 = direction.Cross(axis1);
	axis2.Normalize();

	// Generate vertices for the cylindrical part (circles at both ends)
	std::vector<Vector3> p1Vertices = GenerateCircleVertices(p1, radius, segments, axis1, axis2);
	std::vector<Vector3> p2Vertices = GenerateCircleVertices(p2, radius, segments, axis1, axis2);

	// Draw lines for the cylindrical part (circles and connecting lines)
	for (int i = 0; i < segments; ++i) {
		drawLine->SetLine(p1Vertices[i], p1Vertices[(i + 1) % segments], color);
		drawLine->SetLine(p2Vertices[i], p2Vertices[(i + 1) % segments], color);
		drawLine->SetLine(p1Vertices[i], p2Vertices[i], color);
	}

	// Generate vertices for the half-spheres at both ends in cross pattern
	Vector3 axis3 = Cross(axis1, axis2).Normalized();

	std::vector<Vector3> xyTopHalfSphereVertices = GenerateHalfSphereVertices(p1, radius, segments, axis1, -axis3);
	std::vector<Vector3> zyTopHalfSphereVertices = GenerateHalfSphereVertices(p1, radius, segments, axis2, -axis3);
	std::vector<Vector3> xyBottomHalfSphereVertices = GenerateHalfSphereVertices(p2, radius, segments, axis1, axis3);
	std::vector<Vector3> xzBottomHalfSphereVertices = GenerateHalfSphereVertices(p2, radius, segments, axis2, axis3);

	// Draw lines for the half-spheres in cross pattern
	for (int i = 0; i < xyTopHalfSphereVertices.size(); i++) {
		drawLine->SetLine(xyTopHalfSphereVertices[i], xyTopHalfSphereVertices[(i + 1) % xyTopHalfSphereVertices.size()], color);
		drawLine->SetLine(zyTopHalfSphereVertices[i], zyTopHalfSphereVertices[(i + 1) % zyTopHalfSphereVertices.size()], color);
		drawLine->SetLine(xyBottomHalfSphereVertices[i], xyBottomHalfSphereVertices[(i + 1) % xyBottomHalfSphereVertices.size()], color);
		drawLine->SetLine(xzBottomHalfSphereVertices[i], xzBottomHalfSphereVertices[(i + 1) % xzBottomHalfSphereVertices.size()], color);
	}
}
