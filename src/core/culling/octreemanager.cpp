#include "gr-base/core/culling/octreemanager.hpp"

#include "gr-base/core/culling/frustum.hpp"
#include "gr-base/core/debugdraw.hpp"

#include "gr-base/script/transform.hpp"

#include <gr-math/gmath.hpp>

#include <string.h>

// public
void OctreeManager::InsertGlobal(NodeData data)
{
    GetInstance().insertGlobal(data);
}

bool OctreeManager::Insert(NodeID id, NodeData data)
{
    return GetInstance().insert(id, data);
}

bool OctreeManager::Remove(NodeID id, Transform *transform)
{
    return GetInstance().remove(id, transform);
}

void OctreeManager::Foreach(std::function<bool (OctreeNode *)> func)
{
    return GetInstance().for_each(GetInstance().m_root, func);
}

NodeID OctreeManager::GetRoot()
{
    return GetInstance().m_root;
}

void OctreeManager::Render()
{
    return GetInstance().render();
}

// private
OctreeManager &OctreeManager::GetInstance()
{
    static OctreeManager instance;
    return instance;
}


OctreeManager::OctreeManager() : m_nodes(nullptr), m_count(0), m_size(0), m_root(InvalidNode), m_freeID(InvalidNode)
{
    resizeNodes(1024);

    m_root = allocate(Vector3::zero, 64.0f);
}

OctreeManager::~OctreeManager()
{}

void OctreeManager::insertGlobal(NodeData data)
{
    while (!contains(m_root, data.position))
        expandRoot(data.position);

    insert(m_root, data);
}

void OctreeManager::expandRoot(const Vector3& point)
{
    NodeID oldRoot = m_root;
    OctreeNode* oldNode = m_nodes + oldRoot;

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
    OctreeNode* root = m_nodes + newRoot;

    root->divided = true;
    root->count = 0;

    subdivide(newRoot);

    NodeID prev = InvalidNode;
    NodeID child = root->first;
    for (int i = 0; i < 8; ++i)
    {
        if (child == InvalidNode)
            break;

        if (i == childIndex)
        {
            if (prev != InvalidNode)
                m_nodes[prev].next = oldRoot;
            else
                root->first = oldRoot;

            m_nodes[oldRoot].next = m_nodes[child].next;
            m_nodes[oldRoot].parent = newRoot;

            break;
        }

        prev = child;
        child = m_nodes[child].next;
    }

    m_root = newRoot;
}

bool OctreeManager::insert(NodeID id, NodeData data)
{
    if (!contains(id, data.position))
        return false;

    OctreeNode *node = m_nodes + id;

    float radius = node->radius / 2;

    if (node->count < MAX_OBJECT_IN_NODE)
    {
        node->data[node->count] = data;
        node->count++;
        return true;
    } else
    {
        if (!node->divided)
        {
            subdivide(id);

            node = m_nodes + id;

            for (int j = 0; j < node->count; ++j)
            {
                NodeData oldData = node->data[j];

                for (NodeID i = node->first; i != InvalidNode; i = m_nodes[i].next)
                {
                    if (insert(i, oldData))
                        break;
                }
            }

            node->count = 0;
        }

        for (NodeID i=node->first;i!=InvalidNode;i=m_nodes[i].next)
        {
            if (insert(i, data))
            {
                return true;
            }
        }
    }
    return false;
}

bool OctreeManager::remove(NodeID id, Transform *transform)
{
    /*
    OctreeNode *node = m_nodes + id;
    for (int i=0;i<node->count;i++)
    {
        nquery &query = node->data[i];
        if (query.transform == transform)
        {
            node->data[i] = node->data[node->count - 1];
            node->count--;

            NodeID parent = node->parent;
            return node->count > 0 ? true : (parent != InvalidNode ? check(parent) : true);
        }
    }

    if (node->divided)
    {
        for (NodeID i=node->first;i!=InvalidNode;i=m_nodes[i].next)
        {
            if (remove(i, transform))
                return true;
        }
    }
    */
    return false;
}

void OctreeManager::for_each(NodeID id, std::function<bool (OctreeNode *)> &func)
{
    OctreeNode& node = m_nodes[id];
    if (func(&node))
        return;

    for (NodeID i=node.first;i!=InvalidNode;i=m_nodes[i].next)
        for_each(i, func);
}

bool OctreeManager::check(NodeID id)
{
    if (tryCollapse(id))
    {
        if (m_nodes[id].parent != InvalidNode)
        {
            check(m_nodes[id].parent);
        }
        return true;
    }
    return false;
}

bool OctreeManager::tryCollapse(NodeID id)
{
    OctreeNode &node = m_nodes[id];
    if (!node.divided)
        return false;

    for (NodeID i = node.first; i != InvalidNode; i = m_nodes[i].next)
    {
        if (!empty(i))
            return false;
    }

    for (NodeID i = node.first; i != InvalidNode;)
    {
        NodeID next = m_nodes[i].next;
        deallocate(i);
        i = next;
    }

    node.first = InvalidNode;
    node.divided = false;

    return true;
}

void OctreeManager::deallocate(NodeID id)
{
    OctreeNode *node = m_nodes + id;

    if (node->divided)
    {
        for (NodeID i=node->first;i!=InvalidNode;)
        {
            NodeID next = m_nodes[i].next;
            deallocate(i);
            i = next;
        }
    }

    node->first = InvalidNode;
    node->next = m_freeID;
    m_freeID = id;

    m_count--;
}

bool OctreeManager::empty(NodeID id)
{
    OctreeNode &node = m_nodes[id];
    if (node.count > 0)
        return false;

    if (!node.divided)
        return true;

    for (NodeID i = node.first; i != InvalidNode; i = m_nodes[i].next)
    {
        if (!empty(i))
            return false;
    }

    return true;
}

void OctreeManager::render()
{
    render_node(m_root);
}

void OctreeManager::render_node(NodeID id)
{
    OctreeNode &node = m_nodes[id];

    Vector3 position = node.position;
    float radius = node.radius;

    if (!Frustum::IsSphereInFrustum(position, radius))
        return;

    drawWireCube(node.position, node.radius);

    if (node.divided)
    {
        for (NodeID i=node.first;i!=InvalidNode;i=m_nodes[i].next)
            render_node(i);
    }
}

void OctreeManager::drawWireCube(const Vector3 &center, float size)
{
    DebugDraw::DrawLine(center, center + (Vector3::up * 2.0f));

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
    m_freeID = m_nodes[id].next;

    OctreeNode &node = m_nodes[id];
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
    OctreeNode& node = m_nodes[id];
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
    OctreeNode &node = m_nodes[id];

    if (node.first == InvalidNode)
    {
        node.first = first;
    } else
    {
        NodeID sibling = node.first;
        while (m_nodes[sibling].next != InvalidNode)
            sibling = m_nodes[sibling].next;

        m_nodes[sibling].next = first;
    }
}

void OctreeManager::resizeNodes(size_t size)
{
    std::size_t oldsize = m_size;
    std::size_t newsize = oldsize + size;

    OctreeNode *nodes = (OctreeNode *)std::malloc(sizeof(OctreeNode) * newsize);
    if (nodes == nullptr)
        return;

    if (m_nodes != nullptr)
        memcpy(nodes, m_nodes, sizeof(OctreeNode) * oldsize);

    m_nodes = nodes;
    m_size = newsize;

    addFreeListID(oldsize);
}

void OctreeManager::addFreeListID(NodeID first)
{
    NodeID lastID = m_size - 1;
    for (NodeID i=first;i<lastID;i++)
        m_nodes[i].next = i + 1;

    m_nodes[lastID].next = m_freeID;
    m_freeID = first;
}

bool OctreeManager::contains(NodeID id, const Vector3 &p)
{
    OctreeNode &n = m_nodes[id];

    Vector3 min = n.position - Vector3{n.radius, n.radius, n.radius};
    Vector3 max = n.position + Vector3{n.radius, n.radius, n.radius};

    return p.x >= min.x && p.x < max.x &&
           p.y >= min.y && p.y < max.y &&
           p.z >= min.z && p.z < max.z;
}


