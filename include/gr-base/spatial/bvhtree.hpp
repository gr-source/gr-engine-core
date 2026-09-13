#pragma once

#include "gr-base/ecs/default.hpp"

#include "gr-base/spatial/default.hpp"

#include <gr-math/gmath.hpp>
#include <gr-math/aabb.hpp>

#include <vector>

class BVHtree
{
public:
    BVHtree();
    ~BVHtree();

    BVHtree(const BVHtree&) = delete;
    BVHtree& operator=(const BVHtree&) = delete;

    BVHtree(BVHtree&&) = default;
    BVHtree& operator=(BVHtree&&) = default;

    NodeID insert(Entity entity, const AABB &bounds);

    void remove(NodeID id);

    void sync(NodeID id, const AABB& bounds);

    void queryFrustumAABB(std::vector<Entity>& results);

    void renderer();

private:
    struct Node
    {
        NodeID parent =
            INVALID_NODE;

        NodeID right =
            INVALID_NODE;
        NodeID left =
            INVALID_NODE;

        Entity entity =
            INVALID_ENTITY;

        AABB boundary;

        int height = 0;

        bool isLeaf() const
        {
            return right == INVALID_NODE;
        }
    };

    NodeID* m_freeList;

    Node* m_nodes;

    NodeID m_freeID;

    NodeID m_root;

    size_t m_capacity;

    size_t m_count;

    void renderer_node(NodeID id, uint8_t planeMask);

    void removeLeaf(NodeID id);

    void insertLeaf(NodeID id);

    void syncHeirarchy(NodeID id);

    NodeID balance(NodeID id);

    int getHeight(NodeID id) const;

    int getBalance(NodeID id) const;

    void fatten(AABB &bounds);

    NodeID allocate();

    void deallocate(NodeID id);

    void resize();

    void addFreeList(size_t oldCapacity);
};



