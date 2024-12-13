#pragma once
/**
 * @file CollisionManager.h
 * @brief コリジョンマネージャー
 */
#include <memory>
#include <list>

#include "Collider.h"

class CollisionManager {
public:
    static CollisionManager* GetInstance();
    // コライダー追加
    void AddCollider(Collider* collider);
    // コライダー削除
    void DeleteCollider(Collider* collider);
    // コライダークリア
    void ClearCollider() { colliders_.clear(); }

    // コライダーの当たり判定
    void Collision();
private:
    CollisionManager() = default;
    ~CollisionManager() = default;
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
    CollisionManager(CollisionManager&&) = delete;
    CollisionManager& operator=(CollisionManager&&) = delete;

    std::list<Collider*> colliders_;
};