
#pragma once

#include "gr-base/core/culling/octree.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/default.hpp"

#include "gr-base/script/default.hpp"

#include <functional>

extern "C"
{
#include "gr-base/externals/array.h"
}

using NodeID = std::size_t;

constexpr NodeID InvalidNode = static_cast<NodeID>(-1);

#define MAX_OBJECT_IN_NODE 8

typedef struct NodeData
{
    GameObjectID gameObjectID;
    array* batchIDs;
    Vector3 position;

    array* batchPerLOD;
} NodeData;

typedef struct OctreeNode
{
    NodeID parent;

    NodeID first;

    NodeID next;

    bool divided;

    NodeData data[MAX_OBJECT_IN_NODE];

    uint8_t count;

    Vector3 position;

    float radius;
} OctreeNode;

class OctreeManager
{
public:
    static void Initialize();

    static void Shutdown();

    static NodeID InsertGlobal(NodeData data);

    static bool Insert(NodeID id, NodeData data);

    static bool Remove(NodeID id, GameObjectID handle);

    static NodeID Get(GameObjectID handle, const Vector3& position);

    static void Foreach(std::function<bool (OctreeNode *)> func);

    static NodeID GetRoot();

    static void Render();

private:
    static OctreeManager* s_OctreeManager;

    OctreeManager();
    ~OctreeManager();

    array* m_nodes;

    std::size_t m_count;

    NodeID m_root;

    NodeID m_freeID;

    NodeID insertGlobal(NodeData data);

    void expandRoot(const Vector3& point);

    NodeID insert(NodeID id, NodeData data);

    bool remove(NodeID id, GameObjectID handle);

    bool tryCollapse(NodeID id);

    NodeID get(NodeID id, GameObjectID handle, const Vector3& position);

    void for_each(NodeID id, std::function<bool (OctreeNode *)> &func);

    void deallocate(NodeID id);

    void render();

    void render_node(NodeID id);

    void drawWireCube(const Vector3 &center, float size);

    NodeID allocate(const Vector3 &position, float radius, NodeID parent = InvalidNode);

    void subdivide(NodeID id);

    void emplace(NodeID id, NodeID child);

    void resizeNodes(size_t size);

    void addFreeListID(NodeID first);

    bool contains(NodeID id, const Vector3 &p);
};


