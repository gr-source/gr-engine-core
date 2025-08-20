
#pragma once

#include "gr-base/core/culling/octree.hpp"

#include "gr-base/script/default.hpp"
#include <functional>

using NodeID = std::size_t;

constexpr NodeID InvalidNode = static_cast<NodeID>(-1);

#define MAX_OBJECT_IN_NODE 4

typedef struct nquery
{
    Transform *transform;

    NodeID id;
} nquery;

typedef struct OctreeNode
{
    NodeID parent;

    NodeID first;

    NodeID next;

    bool divided;

    nquery data[MAX_OBJECT_IN_NODE];

    std::size_t count;

    Vector3 position;

    float radius;
} OctreeNode;

class OctreeManager
{
public:
    static bool Insert(NodeID id, Transform *transform);

    static bool Remove(NodeID id, Transform *transform);

    static void Foreach(std::function<bool (OctreeNode *)> func);

    static NodeID GetRoot();

    static void Render();

private:
    static OctreeManager &GetInstance();

    OctreeManager();
    ~OctreeManager();

    OctreeNode *m_nodes;

    std::size_t m_count;

    std::size_t m_size;

    NodeID m_root;

    NodeID m_freeID;

    bool insert(NodeID id, Transform *transform);

    bool remove(NodeID id, Transform *transform);

    void for_each(NodeID id, std::function<bool (OctreeNode *)> &func);

    bool check(NodeID id);

    bool tryCollapse(NodeID id);

    void deallocate(NodeID id);

    bool empty(NodeID id);

    void render();

    void render_node(NodeID id);

    void drawWireCube(const Vector3 &center, float size);

    NodeID create(const Vector3 &position, float radius, NodeID parent = InvalidNode);

    void subdivide(NodeID id);

    void emplace(NodeID id, NodeID child);

    void resize(std::size_t size);

    void addfreelist(NodeID id);

    bool contains(NodeID id, const Vector3 &point);
};


