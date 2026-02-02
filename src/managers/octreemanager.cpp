#include "gr-base/managers/octreemanager.hpp"

#include "gr-base/core/culling/frustum.hpp"
#include "gr-base/core/debugdraw.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/externals/array.h"
#include "gr-base/script/transform.hpp"

#include <gr-math/gmath.hpp>

#include <string.h>

OctreeManager* OctreeManager::s_OctreeManager = nullptr;

// public
/*
void OctreeManager::Initialize()
{
    if (s_OctreeManager == nullptr)
        s_OctreeManager = new OctreeManager();
}

void OctreeManager::Shutdown()
{
    if (s_OctreeManager != nullptr)
        delete s_OctreeManager;
    s_OctreeManager = nullptr;
}
*/

void OctreeManager::SetInstance(OctreeManager *octreeManager)
{
    s_OctreeManager = octreeManager;
}

NodeID OctreeManager::InsertGlobal(NodeData data)
{
    return s_OctreeManager->insertGlobal(data);
}

bool OctreeManager::Insert(NodeID id, NodeData data)
{
    return s_OctreeManager->insert(id, data);
}

bool OctreeManager::Remove(NodeID id, GameObjectID handle)
{
    return s_OctreeManager->remove(id, handle);
}

NodeID OctreeManager::Get(GameObjectID handle, const Vector3& position)
{
    return s_OctreeManager->get(s_OctreeManager->m_root, handle, position);
}

void OctreeManager::Foreach(std::function<bool (OctreeNode *)> func)
{
    return s_OctreeManager->for_each(s_OctreeManager->m_root, func);
}

NodeID OctreeManager::GetRoot()
{
    return s_OctreeManager->m_root;
}

void OctreeManager::Render()
{
    return s_OctreeManager->render();
}

// private
OctreeManager::OctreeManager() : m_nodes(nullptr), m_count(0), m_root(InvalidNode), m_freeID(InvalidNode)
{
    resizeNodes(1024);

    m_root = allocate(Vector3::zero, 64.0f);
}

OctreeManager::~OctreeManager()
{
    if (m_nodes != nullptr)
        array_free(m_nodes);
}

NodeID OctreeManager::insertGlobal(NodeData data)
{
    while (!contains(m_root, data.position))
        expandRoot(data.position);

    return insert(m_root, data);
}

void OctreeManager::expandRoot(const Vector3& point)
{
    NodeID oldRoot = m_root;

    OctreeNode* oldNode = (OctreeNode*)m_nodes->vector + oldRoot;

    int x = point.x >= oldNode->position.x ? 1 : 0;
    int y = point.y >= oldNode->position.y ? 1 : 0;
    int z = point.z >= oldNode->position.z ? 1 : 0;

    int childIndex = (x << 2) | (y << 1) | z;

    float oldR = oldNode->radius;
    float newR = oldR * 2.0f;

    Vector3 newCenter = oldNode->position;
    newCenter.x += (x ? 1 : -1) * oldR;
    newCenter.y += (y ? 1 : -1) * oldR;
    newCenter.z += (z ? 1 : -1) * oldR;

    NodeID newRoot = allocate(newCenter, newR);
    OctreeNode* root = (OctreeNode*)m_nodes->vector + newRoot;

    root->divided = true;
    root->count = 0;

    subdivide(newRoot);

    OctreeNode* nodes = (OctreeNode*)m_nodes->vector;

    NodeID prev = InvalidNode;
    NodeID child = root->first;
    for (int i = 0; i < 8; ++i)
    {
        if (child == InvalidNode)
            break;

        if (i == childIndex)
        {
            if (prev != InvalidNode)
                nodes[prev].next = oldRoot;
            else
                root->first = oldRoot;

            nodes[oldRoot].next = nodes[child].next;
            nodes[oldRoot].parent = newRoot;

            break;
        }

        prev = child;
        child = nodes[child].next;
    }

    m_root = newRoot;
}

NodeID OctreeManager::insert(NodeID id, NodeData data)
{
    if (!contains(id, data.position))
        return InvalidNode;

    OctreeNode *node = ((OctreeNode*)m_nodes->vector) + id;

    float radius = node->radius;

    if (!node->divided && node->count < MAX_OBJECT_IN_NODE)
    {
        node->data[node->count++] = data;

        // std::cout << "Insert in: " << id << " count: " << (int)node->count << " Obj: " << data.gameObjectID << std::endl;
        return id;
    } else if (radius > 1.0f)
    {
        if (!node->divided)
        {
            subdivide(id);

            OctreeNode* nodes = (OctreeNode*)m_nodes->vector;

            node = nodes + id;

            for (int j = 0; j < node->count; ++j)
            {
                NodeData oldData = node->data[j];

                for (NodeID i = node->first; i != InvalidNode; i = nodes[i].next)
                {
                    if (insert(i, oldData) != InvalidNode)
                        break;
                }
            }

            node->count = 0;
        }

        for (NodeID i=node->first;i!=InvalidNode;i=((OctreeNode*)m_nodes->vector)[i].next)
        {
            NodeID childID = insert(i, data);
            if (childID != InvalidNode)
                return childID;
        }
    }
    return InvalidNode;
}

bool OctreeManager::remove(NodeID id, GameObjectID handle)
{
    OctreeNode* nodes = (OctreeNode*)m_nodes->vector;
    OctreeNode& node = nodes[id];

    /*
     * WTF bro ?
    if (!node.divided)
        return false;
    */

    for (uint8_t i = 0; i < node.count; ++i)
    {
        if (node.data[i].gameObjectID == handle)
        {
            node.data[i] = node.data[node.count - 1];
            node.count--;

            NodeID p = node.parent;
            while (p != InvalidNode)
            {
                if (!tryCollapse(p))
                    break;
                p = nodes[p].parent;
            }

            return true;
        }
    }

    return false;
}

bool OctreeManager::tryCollapse(NodeID id)
{
    OctreeNode* nodes = (OctreeNode*)m_nodes->vector;
    OctreeNode& node = nodes[id];

    if (!node.divided)
        return false;

    uint32_t totalCount = 0;
    for (NodeID i = node.first; i != InvalidNode; i = nodes[i].next)
    {
        totalCount += static_cast<uint32_t>(nodes[i].count);

        if (nodes[i].divided)
            return false;
    }

    if (totalCount > MAX_OBJECT_IN_NODE)
        return false;

    node.count = 0;
    for (NodeID i = node.first; i != InvalidNode;)
    {
        OctreeNode& child = nodes[i];
        for (uint8_t j = 0; j < child.count; j++)
        {
            node.data[node.count++] = child.data[j];

            // std::cout << "Col: " << (int)node.count << std::endl;
        }

        auto next = nodes[i].next;

        deallocate(i);

        i = next;
    }

    node.first   = InvalidNode;
    node.divided = false;
    return true;
}

NodeID OctreeManager::get(NodeID id, GameObjectID handle, const Vector3& position)
{
    if (!contains(id, position))
        return InvalidNode;

    OctreeNode* nodes = (OctreeNode*)m_nodes->vector;

    OctreeNode& node = nodes[id];

    for (uint8_t i=0;i<node.count;i++)
    {
        NodeData& data = node.data[i];
        if (data.gameObjectID == handle)
            return id;
    }

    if (node.divided)
    {
        for (NodeID sibling=node.first;sibling!=InvalidNode;sibling=nodes[sibling].next)
        {
            NodeID childID = get(sibling, handle, position);
            if (childID != InvalidNode)
                return childID;
        }
    }

    return InvalidNode;
}

void OctreeManager::for_each(NodeID id, std::function<bool (OctreeNode *)> &func)
{
    OctreeNode& node = ((OctreeNode*)m_nodes->vector)[id];
    if (func(&node))
        return;

    if (node.divided)
    {
        for (NodeID i=node.first;i!=InvalidNode;i=((OctreeNode*)m_nodes->vector)[i].next)
            for_each(i, func);
    }
}

void OctreeManager::deallocate(NodeID id)
{
    OctreeNode *node = ((OctreeNode*)m_nodes->vector) + id;

    node->next = m_freeID;
    m_freeID = id;

    m_count--;
}

void OctreeManager::render()
{
    render_node(m_root);
}

void OctreeManager::render_node(NodeID id)
{
    OctreeNode* nodes = (OctreeNode*)m_nodes->vector;

    OctreeNode &node = nodes[id];

    Vector3 position = node.position;
    float radius = node.radius;

    Vector3 min = position - Vector3{radius, radius, radius};
    Vector3 max = position + Vector3{radius, radius, radius};

    if (!Frustum::IsCubeInFrustumAABB(min, max))
        return;

    drawWireCube(node.position, node.radius);

    if (node.divided)
    {
        for (NodeID i=node.first;i!=InvalidNode;i=nodes[i].next)
            render_node(i);
    }
}

void OctreeManager::drawWireCube(const Vector3 &center, float size)
{
    float half = size;

    // 8 vértices do cubo
    Vector3 v[8] = {
        center + Vector3{-half, -half, -half},
        center + Vector3{ half, -half, -half},
        center + Vector3{ half,  half, -half},
        center + Vector3{-half,  half, -half},
        center + Vector3{-half, -half,  half},
        center + Vector3{ half, -half,  half},
        center + Vector3{ half,  half,  half},
        center + Vector3{-half,  half,  half}
    };

    // Linhas do cubo (12 arestas)
    DebugDraw::DrawLine(v[0], v[1]);
    DebugDraw::DrawLine(v[1], v[2]);
    DebugDraw::DrawLine(v[2], v[3]);
    DebugDraw::DrawLine(v[3], v[0]);

    DebugDraw::DrawLine(v[4], v[5]);
    DebugDraw::DrawLine(v[5], v[6]);
    DebugDraw::DrawLine(v[6], v[7]);
    DebugDraw::DrawLine(v[7], v[4]);

    DebugDraw::DrawLine(v[0], v[4]);
    DebugDraw::DrawLine(v[1], v[5]);
    DebugDraw::DrawLine(v[2], v[6]);
    DebugDraw::DrawLine(v[3], v[7]);
}

NodeID OctreeManager::allocate(const Vector3 &position, float radius, NodeID parent)
{
    if (m_freeID == InvalidNode)
        resizeNodes(1024);

    NodeID id = m_freeID;
    m_freeID = ((OctreeNode*)m_nodes->vector)[id].next;

    OctreeNode &node = ((OctreeNode*)m_nodes->vector)[id];
    node.parent = parent;
    node.first = InvalidNode;
    node.next = InvalidNode;
    node.divided = false;
    node.count = 0;
    node.position = position;
    node.radius = radius;

    m_count++;

    return id;
}

void OctreeManager::subdivide(NodeID id)
{
    OctreeNode& node = ((OctreeNode*)m_nodes->vector)[id];
    node.divided = true;

    Vector3 c = node.position;
    float r = node.radius * 0.5f;

    Vector3 centers[8] = {
        {c.x - r, c.y - r, c.z - r},
        {c.x + r, c.y - r, c.z - r},
        {c.x - r, c.y + r, c.z - r},
        {c.x + r, c.y + r, c.z - r},
        {c.x - r, c.y - r, c.z + r},
        {c.x + r, c.y - r, c.z + r},
        {c.x - r, c.y + r, c.z + r},
        {c.x + r, c.y + r, c.z + r},
    };

    for (int i = 0; i < 8; ++i)
    {
        NodeID child = allocate(centers[i], r, id);
        emplace(id, child);
    }
}

void OctreeManager::emplace(NodeID id, NodeID first)
{
    OctreeNode &node = ((OctreeNode*)m_nodes->vector)[id];

    if (node.first == InvalidNode)
    {
        node.first = first;
    } else
    {
        NodeID sibling = node.first;
        while (((OctreeNode*)m_nodes->vector)[sibling].next != InvalidNode)
            sibling = ((OctreeNode*)m_nodes->vector)[sibling].next;

        ((OctreeNode*)m_nodes->vector)[sibling].next = first;
    }
}

void OctreeManager::resizeNodes(size_t size)
{
    std::size_t oldsize = m_nodes != nullptr ? m_nodes->length : 0;
    std::size_t newsize = oldsize + size;

    array* nodes = array_create(newsize, sizeof(OctreeNode));
    if (nodes == nullptr)
        return;

    if (m_nodes != nullptr)
    {
        memcpy(nodes->vector, m_nodes->vector, sizeof(OctreeNode) * oldsize);

        array_free(m_nodes);
    }

    m_nodes = nodes;

    addFreeListID(oldsize);
}

void OctreeManager::addFreeListID(NodeID first)
{
    OctreeNode* nodes = (OctreeNode*)m_nodes->vector;

    NodeID lastID = m_nodes->length - 1;
    for (NodeID i=first;i<lastID;i++)
        nodes[i].next = i + 1;

    nodes[lastID].next = m_freeID;
    m_freeID = first;
}

bool OctreeManager::contains(NodeID id, const Vector3 &p)
{
    OctreeNode &n = ((OctreeNode*)m_nodes->vector)[id];

    Vector3 min = n.position - Vector3{n.radius, n.radius, n.radius};
    Vector3 max = n.position + Vector3{n.radius, n.radius, n.radius};

    return p.x >= min.x && p.x < max.x &&
           p.y >= min.y && p.y < max.y &&
           p.z >= min.z && p.z < max.z;
}


