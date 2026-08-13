#pragma once

#include "gr-base/ecs/default.hpp"

#include "gr-base/spatial/default.hpp"

#include <gr-math/gmath.hpp>
#include <gr-math/aabb.hpp>

#include <vector>

struct Node
{
    NodeID parent;

    NodeID right;
    NodeID left;

    Entity entity;

    AABB boundary;

    int height;

    inline bool isLeaf() const
    {
        return right == INVALID_NODE;
    }
};

class BVHtree
{
public:
    BVHtree();
    ~BVHtree();

    NodeID insert(Entity entity, const AABB &bounds);

    void remove(NodeID id);

    void sync(NodeID id, const AABB& bounds);

    void query(const AABB& queryBounds, std::vector<Entity>& results);

    void queryFrustumAABB(std::vector<Entity>& results);

    void renderer();

private:
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



