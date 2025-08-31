#include "gr-base/broadphase/bvhtree.hpp"
#include "gr-base/broadphase/default.hpp"
#include "gr-base/core/debugdraw.hpp"
#include <cstring>
#include <gr-math/gmath.hpp>

bvh::Bounds Union(const bvh::Bounds &a, const bvh::Bounds &b)
{
    bvh::Bounds c;
	c.min = Math::min(a.min, b.min);
	c.max = Math::max(a.max, b.max);

    return c;
}

namespace bvh
{
    // public
    NodeID Tree::Insert(const Bounds &bounds, void *data)
    {
        return GetInstance().insert(bounds, data);
    }

    void Tree::Remove(NodeID id)
    {
        return GetInstance().remove(id);
    }

    void Tree::For_each(std::function<bool (struct Node<void *> *)> func)
    {
        auto &instance = GetInstance();

        return instance.for_each(instance.m_root, func);
    }

    void Tree::Render()
    {
        return GetInstance().render();
    }

    // private
    Tree &Tree::GetInstance()
    {
        static Tree instance;
        return instance;
    }

    Tree::Tree() : m_freeID(Null), m_root(Null), m_nodes(nullptr), m_size(0), m_count(0)
    {}

    Tree::~Tree()
    {
        if (m_nodes == nullptr)
            return;

        free(m_nodes);
    }

    NodeID Tree::insert(const Bounds &bounds, void *data)
    {
        NodeID id = allocate();

        struct Node<void *> &node = m_nodes[id];
        node.boundary = bounds;
        node.data = data;

        fatten(node.boundary);

        insertLeaf(id);

        return id;
    }

    void Tree::remove(NodeID id)
    {
        struct Node<void *> &node = m_nodes[id];
        if (!node.isLeaf())
        {
            return;
        }

        removeLeaf(id);

        deallocate(id);
    }

    void Tree::for_each(NodeID id, std::function<bool (struct Node<void *> *)> func)
    {
        if (id == Null)
            return;

        struct Node<void *> *node = m_nodes + id;
        if (func(node))
            return;

        for_each(node->right, func);
        for_each(node->left, func);
    }

    void Tree::render()
    {
        if (m_root != Null)
            render_node(m_root);
    }

    void Tree::render_node(NodeID id)
    {
        struct Node<void *> &node = m_nodes[id];

        const Bounds& b = node.boundary;
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
            render_node(node.right);
            render_node(node.left);
        }
    }

    NodeID Tree::allocate()
    {
        if (m_freeID == Null)
            resize(1024);

        NodeID id = m_freeID;
        m_freeID = m_nodes[id].next;

        struct Node<void *> &node = m_nodes[id];
        node.parent = Null;
        node.right = Null;
        node.left = Null;
        node.height = 0;

        m_count++;

        return id;
    }

    void Tree::deallocate(NodeID id)
    {
        struct Node<void *> &node = m_nodes[id];

        node.next = m_freeID;
        m_freeID = id;

        m_count--;
    }

    void Tree::resize(const std::size_t &size)
    {
        auto oldsize = m_size;
        auto newsize = oldsize + size;

        struct Node<void *> *temp = (struct Node<void *> *)malloc(sizeof(struct Node<void *>) * newsize);
        if (m_nodes != nullptr)
        {
            memcpy(temp, m_nodes, sizeof(struct Node<void *>) * oldsize);
            free(m_nodes);
        }

        m_nodes = temp;
        m_size = newsize;

        addfreeid(oldsize);
    }

    void Tree::addfreeid(NodeID id)
    {
        for (NodeID i=id;i<m_size - 1;i++)
        {
            m_nodes[i].next = i + 1;
        }

        m_nodes[m_size - 1].next = m_freeID;
        m_freeID = id;
    }

    void Tree::fatten(Bounds &bounds)
    {
        float value = 0.5f;
        bounds.min -= value;
        bounds.max += value;
    }

    void Tree::insertLeaf(NodeID id)
    {
        if (m_root == Null)
        {
            m_root = id;
            return;
        }

        NodeID current = m_root;
        auto leafrect = m_nodes[id].boundary;
        while (!m_nodes[current].isLeaf())
        {
            auto boundary = Union(leafrect, m_nodes[current].boundary);
            float UniondArea = boundary.area();
            float branchCost = 2.0f * UniondArea;

            float inheritedCost = 2.0f * (UniondArea - m_nodes[current].boundary.area());

            NodeID left = m_nodes[current].left;
            NodeID right = m_nodes[current].right;

            float leftDescentCost;
            if (m_nodes[left].isLeaf())
            {
                leftDescentCost = Union(leafrect, m_nodes[left].boundary).area() + inheritedCost;
            } else
            {
                float inflated = Union(leafrect, m_nodes[left].boundary).area();
                float branchArea = m_nodes[left].boundary.area();
                leftDescentCost = inflated - branchArea + inheritedCost;
            }

            float rightDescentCost;
            if (m_nodes[right].isLeaf())
            {
                rightDescentCost = Union(leafrect, m_nodes[right].boundary).area() + inheritedCost;
            } else
            {
                float inflated = Union(leafrect, m_nodes[right].boundary).area();
                float branchArea = m_nodes[right].boundary.area();
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

        m_nodes[newParent].boundary = Union(leafrect, m_nodes[sibling].boundary);
        m_nodes[newParent].parent = oldParent;
        m_nodes[newParent].height = m_nodes[sibling].height + 1;

        if (oldParent != Null)
        {
            // pai antigo
            {
                struct Node<void *> &parent = m_nodes[oldParent];
                if (parent.left == sibling)
                    parent.left = newParent;
                else
                    parent.right = newParent;
            }

            struct Node<void *> &parent = m_nodes[newParent];
            parent.left = sibling;
            parent.right = id;

            m_nodes[sibling].parent = newParent;
            m_nodes[id].parent = newParent;
        } else
        {
            struct Node<void *> &parent = m_nodes[newParent];
            parent.left = sibling;
            parent.right = id;

            m_nodes[sibling].parent = newParent;
            m_nodes[id].parent = newParent;
            m_root = newParent;
        }

        syncHeirarchy(m_nodes[id].parent);
    }


        void Tree::removeLeaf(NodeID id)
        {
            if (m_root == id)
            {
                m_root = Null;
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
            if ( grandParent != Null)
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
                m_nodes[sibling].parent = Null;
            }

            deallocate(parent);
            syncHeirarchy(grandParent);
        }

    void Tree::syncHeirarchy(NodeID id)
    {
        while (id != Null)
        {
            id = balance(id);

            NodeID left = m_nodes[id].left;
            NodeID right = m_nodes[id].right;

            m_nodes[id].height = 1 + std::max(m_nodes[left].height, m_nodes[right].height);
            m_nodes[id].boundary = Union(m_nodes[left].boundary, m_nodes[right].boundary);

            id = m_nodes[id].parent;
        }
    }


    NodeID Tree::balance(NodeID iA)
    {
	    struct Node<void *> &A = m_nodes[iA];

	    if (A.isLeaf() || A.height == 1)
		    return iA;
        
	    NodeID iB = A.left;
	    NodeID iC = A.right;
	    struct Node<void *> &B = m_nodes[iB]; // left
	    struct Node<void *> &C = m_nodes[iC]; // right
        

        int balance = getBalance(iA);

	    // right rotate
	    if (balance < -1)
	    {
		    NodeID iF = C.left;
		    NodeID iG = C.right;
		    struct Node<void *> &F = m_nodes[iF];
		    struct Node<void *> &G = m_nodes[iG];

		    // grandParent point to C
		    if (A.parent != Null)
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
			    A.boundary = Union(B.boundary, G.boundary);
			    C.boundary = Union(A.boundary, F.boundary);

			    A.height = 1 + std::max(B.height, G.height);
			    C.height = 1 + std::max(A.height, F.height);
		    } else
		    {
			    C.right = iG;
			    A.right = iF;
			    F.parent = iA;
			    A.boundary = Union(B.boundary, F.boundary);
			    C.boundary = Union(A.boundary, G.boundary);

			    A.height = 1 + std::max(B.height, F.height);
			    C.height = 1 + std::max(A.height, G.height);
		    }

		    return iC;
	    } else if (balance > 1)
	    {
		    NodeID iD = B.left;
		    NodeID iE = B.right;
		    struct Node<void *> &D = m_nodes[iD];
		    struct Node<void *> &E = m_nodes[iE];

		    // grandParent point to B
		    if (A.parent != Null)
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
			    A.boundary = Union(C.boundary, E.boundary);
			    B.boundary = Union(A.boundary, D.boundary);

			    A.height = 1 + std::max(C.height, E.height);
			    B.height = 1 + std::max(A.height, D.height);
		    } else
		    {
			    B.left = iE;
			    A.left = iD;
			    D.parent = iA;
			    A.boundary = Union(C.boundary, D.boundary);
			    B.boundary = Union(A.boundary, E.boundary);

			    A.height = 1 + std::max(C.height, D.height);
			    B.height = 1 + std::max(A.height, E.height);
		    }

		    return iB;
        }
    
        return iA;
    }

    int Tree::getHeight(NodeID id) const
    {
        return id != Null ? m_nodes[id].height : 0;
    }

    int Tree::getBalance(NodeID id) const
    {
        return getHeight(m_nodes[id].left) - getHeight(m_nodes[id].right);
    }

};
