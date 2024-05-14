#pragma once

#include <memory>
#include <list>

#include "Collider.h"

class CollisionManager {
public:
    static CollisionManager* GetInstance();

    void AddCollider(Collider* collider);
    void DeleteCollider(Collider* collider);
    void ClearCollider() { colliders_.clear(); }

    void Collision();
private:
    CollisionManager() = default;
    ~CollisionManager() = default;
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
    CollisionManager(CollisionManager&&) = delete;
    CollisionManager& operator=(CollisionManager&&) = delete;

    std::list<std::unique_ptr<Collider>> colliders_;
};