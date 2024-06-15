#include "CollisionManager.h"

CollisionManager* CollisionManager::GetInstance() {
    static CollisionManager instance;
    return &instance;
}

void CollisionManager::AddCollider(Collider* collider) {
    colliders_.emplace_back(collider);
}

void CollisionManager::DeleteCollider(Collider* collider) {
    auto iter = std::find_if(colliders_.begin(), colliders_.end(),
        [collider](const std::unique_ptr<Collider>& ptr) { return ptr.get() == collider; });
    if (iter != colliders_.end()) {
        colliders_.erase(iter);
    }
}

void CollisionManager::Collision() {
    auto iter1 = colliders_.begin();
    for (; iter1 != colliders_.end(); ++iter1) {
        Collider* collider1 = iter1->get();  

        // アクティブじゃなければ通さない
        if (!collider1->GetIsActive()) { continue; }

        auto iter2 = iter1;
        ++iter2;
        for (; iter2 != colliders_.end(); ++iter2) {
            Collider* collider2 = iter2->get();

            // アクティブじゃなければ通さない
            if (!collider2->GetIsActive()) { continue; }

            ColliderDesc colliderDesc1{};
            if (collider1->IsCollision(collider2, colliderDesc1)) {
                // 衝突情報を反転
                ColliderDesc colliderDesc2 = colliderDesc1;
                colliderDesc2.collider = collider1;
                colliderDesc2.normal = -colliderDesc1.normal;
                // Stayを呼び出す
                collider1->OnCollision(colliderDesc1);
                collider2->OnCollision(colliderDesc2);
            }
        }
    }
}

