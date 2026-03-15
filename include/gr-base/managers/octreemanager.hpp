#pragma once

#include "gr-base/ecs/default.hpp"

#include "default.hpp"

#include <functional>

extern "C"
{
#include "gr-base/externals/array.h"
}

#define MAX_OBJECT_IN_NODE 8

typedef struct NodeData
{
    GameObjectHandle gameObjectID;
    array* batchIDs;
    Boundbox bounds;

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

    Boundbox boundary;
} OctreeNode;

class OctreeManager
{
public:
    static void SetInstance(OctreeManager* octreeManager);

    static NodeID InsertGlobal(NodeData data);

    static bool Insert(NodeID id, NodeData data);

    static bool Remove(NodeID id, GameObjectHandle handle);

    static NodeID Get(GameObjectHandle handle, const Vector3& position);

    static void Foreach(std::function<bool (OctreeNode *)> func);

    static NodeID GetRoot();

    static void Render();

    OctreeManager();
    ~OctreeManager();

private:
    static OctreeManager* s_OctreeManager;

    array* m_nodes;

    std::size_t m_count;

    NodeID m_root;

    NodeID m_freeID;

    NodeID insertGlobal(NodeData data);

    void expandRoot(const Boundbox& bounds);

    NodeID insert(NodeID id, NodeData data);

    bool remove(NodeID id, GameObjectHandle handle);

    bool tryCollapse(NodeID id);

    NodeID get(NodeID id, GameObjectHandle handle, const Vector3& position);

    void for_each(NodeID id, std::function<bool (OctreeNode *)> &func);

    void deallocate(NodeID id);

    void render();

    void render_node(NodeID id);

    void drawWireCube(const Boundbox& boundary);

    NodeID allocate(const Boundbox& boundary, NodeID parent = InvalidNode);

    void subdivide(NodeID id);

    void emplace(NodeID id, NodeID child);

    void resizeNodes(size_t size);

    void addFreeListID(NodeID first);

    bool contains(NodeID id, const Vector3& p);

    bool contains(NodeID id, const Boundbox& b);
};


