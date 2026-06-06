#include "gr-base/spatial/bvhtree.hpp"

#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/debug/debugDraw.hpp"

#include <cstdlib>

BVHtree::BVHtree() : m_freeList(nullptr), m_freeID(INVALID_NODE), m_nodes(nullptr), m_capacity(0), m_root(INVALID_NODE)
{}

BVHtree::~BVHtree()
{
    if (m_freeList != nullptr)
        free(m_freeList);

    if (m_nodes != nullptr)
        free(m_nodes);
}

void BVHtree::insert(Entity entity, const AABB &bounds)
{
    uint32_t entityID = handle_get_first(entity);
    if (entityID >= m_entityToNode.size())
        m_entityToNode.resize(entityID + 1, INVALID_NODE);

    NodeID id = allocate();

    Node& node = m_nodes[id];
    node.boundary = bounds;
    node.entity = entity;

    fatten(node.boundary);

    insertLeaf(id);

    m_entityToNode[entityID] = id;
}

void BVHtree::remove(Entity entity)
{
    uint32_t entityID = handle_get_first(entity);
    if (entityID >= m_entityToNode.size())
        return;

    NodeID id = m_entityToNode[entityID];
    assert(id != INVALID_NODE);

    Node &node = m_nodes[id];
    if (!node.isLeaf())
        return;

    removeLeaf(id);

    deallocate(id);

    m_entityToNode[entityID] = INVALID_NODE;
}

void BVHtree::sync(Entity entity, const AABB& bounds)
{
    uint32_t entityID = handle_get_first(entity);
    if (entityID >= m_entityToNode.size())
        return;

    NodeID id = m_entityToNode[entityID];
    if (id == INVALID_NODE)
        return;

    Node& node = m_nodes[id];
    node.boundary = bounds;

    if (!node.isLeaf())
        syncHeirarchy(id);
    else
        syncHeirarchy(node.parent);
}

#include <stack>

void BVHtree::query(const AABB& queryBounds, std::vector<Entity>& results)
{
    std::stack<NodeID> stack;
    stack.push(m_root);

    while (!stack.empty())
    {
        NodeID id = stack.top();
        stack.pop();

        if (id == INVALID_NODE) continue;
        Node& node = m_nodes[id];

        if (!node.boundary.Intersects(queryBounds))
            continue;

        if (node.isLeaf())
            results.push_back(node.entity);
        else
        {
            stack.push(node.left);
            stack.push(node.right);
        }
    }
}


void BVHtree::queryFrustumAABB(std::vector<Entity>& results)
{
    std::stack<NodeID> stack;
    stack.push(m_root);

    while (!stack.empty())
    {
        NodeID id = stack.top();
        stack.pop();

        if (id == INVALID_NODE) continue;
        Node& node = m_nodes[id];

        auto& boundary = node.boundary;

        if (!Frustum::IsCubeInFrustumAABB(boundary.min, boundary.max))
            continue;

        if (node.isLeaf())
            results.push_back(node.entity);
        else
        {
            stack.push(node.left);
            stack.push(node.right);
        }
    }
}

void BVHtree::renderer()
{
    if (m_root != INVALID_NODE)
        renderer_node(m_root);
}

/*
void BVHtree::for_each(NodeID id, std::function<bool (struct Node<void *> *)> func)
{
    if (id == Null)
        return;

    struct Node<void *> *node = m_nodes + id;
    if (func(node))
        return;

    for_each(node->right, func);
    for_each(node->left, func);
}
*/

// private
void BVHtree::renderer_node(NodeID id)
{
    Node& node = m_nodes[id];

    const AABB& b = node.boundary;
    auto min = b.min;
    auto max = b.max;

    Vector3 points[8] {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {max.x, max.y, min.z},
        {min.x, max.y, min.z},
 
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {max.x, max.y, max.z},
        {min.x, max.y, max.z}
    };

    DebugDraw::DrawLine(points[0], points[1]);
    DebugDraw::DrawLine(points[1], points[2]);
    DebugDraw::DrawLine(points[2], points[3]);
    DebugDraw::DrawLine(points[3], points[0]);

    DebugDraw::DrawLine(points[4], points[5]);
    DebugDraw::DrawLine(points[5], points[6]);
    DebugDraw::DrawLine(points[6], points[7]);
    DebugDraw::DrawLine(points[7], points[4]);

    DebugDraw::DrawLine(points[0], points[4]);
    DebugDraw::DrawLine(points[1], points[5]);
    DebugDraw::DrawLine(points[2], points[6]);
    DebugDraw::DrawLine(points[3], points[7]);

    if (!node.isLeaf())
    {
        renderer_node(node.right);
        renderer_node(node.left);
    }
}

void BVHtree::removeLeaf(NodeID id)
{
    if (m_root == id)
    {
        m_root = INVALID_NODE;
        return;
    }

    // Setup parent, grandParent and sibling
    NodeID parent = m_nodes[id].parent;
    NodeID grandParent = m_nodes[parent].parent;
    NodeID sibling;

    if (m_nodes[parent].left == id)
        sibling = m_nodes[parent].right;
    else
        sibling = m_nodes[parent].left;

    // Remove parent and replace with sibling
    if (grandParent != INVALID_NODE)
    {
        // Connect grandParent to sibling
        if (m_nodes[grandParent].left == parent)
            m_nodes[grandParent].left = sibling;
        else
            m_nodes[grandParent].right = sibling;

        // Connect sibling to grandParent
        m_nodes[sibling].parent = grandParent;
    } else
    {
        m_root = sibling;
        m_nodes[sibling].parent = INVALID_NODE;
    }

    deallocate(parent);
    syncHeirarchy(grandParent);
}

void BVHtree::insertLeaf(NodeID id)
{
    if (m_root == INVALID_NODE)
    {
        m_root = id;
        return;
    }

    NodeID current = m_root;
    auto leafrect = m_nodes[id].boundary;
    while (!m_nodes[current].isLeaf())
    {
        auto boundary = AABB::Merged(leafrect, m_nodes[current].boundary);
        float UniondArea = boundary.SurfaceArea();
        float branchCost = 2.0f * UniondArea;

        float inheritedCost = 2.0f * (UniondArea - m_nodes[current].boundary.SurfaceArea());

        NodeID left = m_nodes[current].left;
        NodeID right = m_nodes[current].right;

        float leftDescentCost;
        if (m_nodes[left].isLeaf())
        {
            leftDescentCost = AABB::Merged(leafrect, m_nodes[left].boundary).SurfaceArea() + inheritedCost;
        } else
        {
            float inflated = AABB::Merged(leafrect, m_nodes[left].boundary).SurfaceArea();
            float branchArea = m_nodes[left].boundary.SurfaceArea();
            leftDescentCost = inflated - branchArea + inheritedCost;
        }

        float rightDescentCost;
        if (m_nodes[right].isLeaf())
        {
            rightDescentCost = AABB::Merged(leafrect, m_nodes[right].boundary).SurfaceArea() + inheritedCost;
        } else
        {
            float inflated = AABB::Merged(leafrect, m_nodes[right].boundary).SurfaceArea();
            float branchArea = m_nodes[right].boundary.SurfaceArea();
            rightDescentCost = inflated - branchArea + inheritedCost;
        }

        if (branchCost < leftDescentCost && branchCost < rightDescentCost)
            break;

        if (leftDescentCost < rightDescentCost)
            current = left;
        else
            current = right;
    }

    NodeID sibling = current;

    NodeID oldParent = m_nodes[sibling].parent;
    NodeID newParent = allocate();

    m_nodes[newParent].boundary = AABB::Merged(leafrect, m_nodes[sibling].boundary);
    m_nodes[newParent].parent = oldParent;
    m_nodes[newParent].height = m_nodes[sibling].height + 1;

    if (oldParent != INVALID_NODE)
    {
        // pai antigo
        {
            Node& parent = m_nodes[oldParent];
            if (parent.left == sibling)
                parent.left = newParent;
            else
                parent.right = newParent;
        }

        Node& parent = m_nodes[newParent];
        parent.left = sibling;
        parent.right = id;

        m_nodes[sibling].parent = newParent;
        m_nodes[id].parent = newParent;
    } else
    {
        Node& parent = m_nodes[newParent];
        parent.left = sibling;
        parent.right = id;

        m_nodes[sibling].parent = newParent;
        m_nodes[id].parent = newParent;
        m_root = newParent;
    }

    syncHeirarchy(m_nodes[id].parent);
}

void BVHtree::syncHeirarchy(NodeID id)
{
    while (id != INVALID_NODE)
    {
        id = balance(id);

        NodeID left = m_nodes[id].left;
        NodeID right = m_nodes[id].right;

        m_nodes[id].height = 1 + std::max(m_nodes[left].height, m_nodes[right].height);
        m_nodes[id].boundary = AABB::Merged(m_nodes[left].boundary, m_nodes[right].boundary);

        id = m_nodes[id].parent;
    }
}


NodeID BVHtree::balance(NodeID iA)
{
    Node& A = m_nodes[iA];

    if (A.isLeaf() || A.height == 1)
        return iA;
    
    NodeID iB = A.left;
    NodeID iC = A.right;
    Node& B = m_nodes[iB]; // left
    Node& C = m_nodes[iC]; // right
    

    int balance = getBalance(iA);

    // right rotate
    if (balance < -1)
    {
        NodeID iF = C.left;
        NodeID iG = C.right;
        Node& F = m_nodes[iF];
        Node& G = m_nodes[iG];

        // grandParent point to C
        if (A.parent != INVALID_NODE)
        {
            if(m_nodes[A.parent].left == iA)
                m_nodes[A.parent].left = iC;
            else
                m_nodes[A.parent].right = iC;
        }
        else
            m_root = iC;

        // Swap A and C
        C.left = iA;
        C.parent = A.parent;
        A.parent = iC;

        // Finish rotation
        if ( F.height > G.height )
        {
            C.right = iF;
            A.right = iG;
            G.parent = iA;
            A.boundary = AABB::Merged(B.boundary, G.boundary);
            C.boundary = AABB::Merged(A.boundary, F.boundary);

            A.height = 1 + std::max(B.height, G.height);
            C.height = 1 + std::max(A.height, F.height);
        } else
        {
            C.right = iG;
            A.right = iF;
            F.parent = iA;
            A.boundary = AABB::Merged(B.boundary, F.boundary);
            C.boundary = AABB::Merged(A.boundary, G.boundary);

            A.height = 1 + std::max(B.height, F.height);
            C.height = 1 + std::max(A.height, G.height);
        }

        return iC;
    } else if (balance > 1)
    {
        NodeID iD = B.left;
        NodeID iE = B.right;
        Node& D = m_nodes[iD];
        Node& E = m_nodes[iE];

        // grandParent point to B
        if (A.parent != INVALID_NODE)
        {
            if(m_nodes[A.parent].left == iA)
                m_nodes[A.parent].left = iB;
            else
                m_nodes[A.parent].right = iB;
        } else
            m_root = iB;

        // Swap A and B
        B.right = iA;
        B.parent = A.parent;
        A.parent = iB;

        // Finish rotation
        if (D.height > E.height)
        {
            B.left = iD;
            A.left = iE;
            E.parent = iA;
            A.boundary = AABB::Merged(C.boundary, E.boundary);
            B.boundary = AABB::Merged(A.boundary, D.boundary);

            A.height = 1 + std::max(C.height, E.height);
            B.height = 1 + std::max(A.height, D.height);
        } else
        {
            B.left = iE;
            A.left = iD;
            D.parent = iA;
            A.boundary = AABB::Merged(C.boundary, D.boundary);
            B.boundary = AABB::Merged(A.boundary, E.boundary);

            A.height = 1 + std::max(C.height, D.height);
            B.height = 1 + std::max(A.height, E.height);
        }

        return iB;
    }

    return iA;
}

int BVHtree::getHeight(NodeID id) const
{
    return id != INVALID_NODE ? m_nodes[id].height : 0;
}

int BVHtree::getBalance(NodeID id) const
{
    return getHeight(m_nodes[id].left) - getHeight(m_nodes[id].right);
}

void BVHtree::fatten(AABB &bounds)
{
    float value = 0.5f;
    bounds.min -= value;
    bounds.max += value;
}

NodeID BVHtree::allocate()
{
    if (m_freeID == INVALID_NODE)
        resize();

    NodeID id = m_freeID;
    m_freeID = m_freeList[id];

    Node& node = m_nodes[id];
    node.parent = INVALID_NODE;
    node.right = INVALID_NODE;
    node.left = INVALID_NODE;
    node.height = 0;

    return id;
}

void BVHtree::deallocate(NodeID id)
{
    m_freeList[id] = m_freeID;
    m_freeID = id;
}

void BVHtree::resize()
{
    size_t oldCapacity = m_capacity;

    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_freeList = (NodeID*)realloc(m_freeList, sizeof(NodeID) * m_capacity);
    m_nodes = (Node*)realloc(m_nodes, sizeof(Node) * m_capacity);

    addFreeList(oldCapacity);
}

void BVHtree::addFreeList(size_t oldCapacity)
{
    size_t lastElement = m_capacity - 1;
    for (size_t i=oldCapacity;i<lastElement;i++)
        m_freeList[i] = i + 1;

    m_freeList[lastElement] = m_freeID;
    m_freeID = static_cast<NodeID>(oldCapacity);
}

