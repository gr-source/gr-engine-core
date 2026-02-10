#pragma once

#include <functional>

#include "default.hpp"

typedef struct BVHNode
{
    NodeID parent;
    NodeID next;

    NodeID right;
    NodeID left;

    void* data;

    Boundbox boundary;
    Boundbox localBoundary;

    int height;

    inline bool isLeaf() const
    {
        return right == InvalidNode;
    }
} BVHNode;

class Tree
{
public:
    static void SetInstance(Tree* instance);

    static NodeID Insert(const Boundbox &bounds, void *data);

    static void Remove(NodeID id);

    static void For_each(std::function<bool (NodeID, BVHNode *)> func);

    static void SyncHeirarchy(NodeID id);

    static void Render();

    Tree();
    ~Tree();

private:
    static Tree *s_Tree;

    NodeID m_freeID;

    NodeID m_root;

    BVHNode* m_nodes;

    std::size_t m_size;

    std::size_t m_count;

    NodeID insert(const Boundbox &bounds, void *data);

    void remove(NodeID id);

    void for_each(NodeID node, std::function<bool (NodeID, BVHNode *)> func);

    void render();

    void render_node(NodeID id);

    NodeID allocate();

    void deallocate(NodeID id);

    void resize(const std::size_t &size);

    void addfreeid(NodeID id);

    void fatten(Boundbox &bounds);

    void insertLeaf(NodeID id);

    void removeLeaf(NodeID id);

    void syncHeirarchy(NodeID id);

    NodeID balance(NodeID id);

    int getHeight(NodeID id) const;

    int getBalance(NodeID id) const;
};

