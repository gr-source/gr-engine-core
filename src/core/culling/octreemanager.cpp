#include "gr-base/core/culling/octreemanager.hpp"

#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/core/debugdraw.hpp"

#include "gr-base/script/camera.hpp"
#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"
#include <gr-math/gmath.hpp>

// public
bool OctreeManager::Insert(NodeID id, Transform *transform)
{
    return GetInstance().insert(id, transform);
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
    resize(1024);

    m_root = create(Vector3::zero, 64.0f);
}

OctreeManager::~OctreeManager()
{}

bool OctreeManager::insert(NodeID id, Transform *transform)
{
    if (!contains(id, transform->getPosition()))
        return false;

    OctreeNode *node = m_nodes + id;
    if (node->count < MAX_OBJECT_IN_NODE)
    {
        node->data[node->count] = {transform, id};
        node->count++;
        return true;
    } else
    {
        if (!node->divided)
        {
            subdivide(id);

            node = m_nodes + id;
        }

        for (NodeID i=node->first;i!=InvalidNode;i=m_nodes[i].next)
        {
            if (insert(i, transform))
            {
                return true;
            }
        }
    }
    return false;
}

bool OctreeManager::remove(NodeID id, Transform *transform)
{
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
    return false;
}

void OctreeManager::for_each(NodeID id, std::function<bool (OctreeNode *)> &func)
{
    OctreeNode *node = m_nodes + id;
    if (func(node))
        return;

    for (NodeID i=node->first;i!=InvalidNode;i=m_nodes[i].next)
    {
        for_each(i, func);
    }
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

    /*
    Camera *camera = Camera::Current();
    if (camera == nullptr)
        return;

    Vector3 position = node.position;
    float radius = node.radius;

    Vector3 min = position - Vector3{radius, radius, radius};
    Vector3 max = position + Vector3{radius, radius, radius};

    if (!Frustum::IsCubeInFrustumAABB(min, max))
        return;
    */

    drawWireCube(node.position, node.radius);

    if (node.divided)
    {
        for (NodeID i=node.first;i!=InvalidNode;i=m_nodes[i].next)
        {
            render_node(i);
        }
    }
}

void OctreeManager::drawWireCube(const Vector3 &center, float size)
{
    DebugDraw::DrawLine(center, center + (Vector3::up * 2));

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

NodeID OctreeManager::create(const Vector3 &position, float radius, NodeID parent)
{
    if (m_freeID == InvalidNode)
        resize(1024);

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

    for (int i=0;i<MAX_OBJECT_IN_NODE;i++)
        node.data[i] = {nullptr, InvalidNode};

    m_count++;

    return id;
}

void OctreeManager::subdivide(NodeID id)
{
    OctreeNode *node = m_nodes + id;
    node->divided = true;

    Vector3 center = node->position;
    float radius = node->radius / 2;

    Vector3 tfr{center.x + radius, center.y - radius, center.z - radius}; // Top Front Right
    Vector3 tfl{center.x - radius, center.y + radius, center.z - radius}; // Top Front Left
    Vector3 tbr{center.x + radius, center.y + radius, center.z + radius}; // Top Back Right
    Vector3 tbl{center.x - radius, center.y + radius, center.z + radius}; // Top Back Left

    Vector3 bfr{center.x + radius, center.y - radius, center.z - radius}; // Bottom Front Right
    Vector3 bfl{center.x - radius, center.y - radius, center.z - radius}; // Bottom Front Left
    Vector3 bbr{center.x + radius, center.y - radius, center.z + radius}; // Bottom Back Right
    Vector3 bbl{center.x - radius, center.y - radius, center.z + radius}; // Bottom Back Left
    
    NodeID id0 = create(tfr, radius, id);
    NodeID id1 = create(tfl, radius, id);
    NodeID id2 = create(tbr, radius, id);
    NodeID id3 = create(tbl, radius, id);
    NodeID id4 = create(bfr, radius, id);
    NodeID id5 = create(bfl, radius, id);
    NodeID id6 = create(bbr, radius, id);
    NodeID id7 = create(bbl, radius, id);

    emplace(id, id0);
    emplace(id, id1);
    emplace(id, id2);
    emplace(id, id3);
    emplace(id, id4);
    emplace(id, id5);
    emplace(id, id6);
    emplace(id, id7);
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

void OctreeManager::resize(std::size_t size)
{
    std::size_t oldsize = m_size;
    std::size_t newsize = oldsize + size;

    OctreeNode *nodes = (OctreeNode *)std::malloc(sizeof(OctreeNode) * newsize);
    if (nodes == nullptr)
        return;

    if (m_nodes != nullptr)
        std::memcpy(nodes, m_nodes, sizeof(OctreeNode) * oldsize);

    m_nodes = nodes;
    m_size = newsize;

    addfreelist(oldsize);
}

void OctreeManager::addfreelist(NodeID id)
{
    for (NodeID i=id;i<m_size-1;i++)
        m_nodes[i].next = i + 1;

    m_nodes[m_size - 1].next = m_freeID;
    m_freeID = id;
}

bool OctreeManager::contains(NodeID id, const Vector3 &point)
{
    OctreeNode &node = m_nodes[id];

    Vector3 position = node.position;

    return (point.x >= position.x - node.radius && point.x <= position.x + node.radius &&
            point.y >= position.y - node.radius && point.y <= position.y + node.radius &&
            point.z >= position.z - node.radius && point.z <= position.z + node.radius);
}


