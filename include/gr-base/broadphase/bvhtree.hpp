#pragma once

#include "default.hpp"

namespace bvh
{
    struct Bounds
    {
        Vector3 min;
        Vector3 max;

        inline float area() const
        {
            Vector3 size = max - min;
            return 2.0f * (size.x * size.y + size.y * size.z + size.z * size.x);
        }
    };

    struct Node
    {
        NodeID parent;
        NodeID next;

        NodeID right;
        NodeID left;

        void *data;

        Bounds boundary;

        int height;

        inline bool isLeaf() const
        {
            return right == Null;
        }
    };

    class Tree
    {
    public:
        static NodeID Insert(const Bounds &bounds, void *data);

        static void Remove(NodeID id);

        static void Render();

    private:
        static Tree &GetInstance();

        NodeID m_freeID;

        NodeID m_root;

        Node *m_nodes;

        std::size_t m_size;

        std::size_t m_count;

        Tree();
        ~Tree();

        NodeID insert(const Bounds &bounds, void *data);

        void remove(NodeID id);

        void render();

        void render_node(NodeID id);

        NodeID allocate();

        void deallocate(NodeID id);

        void resize(const std::size_t &size);

        void addfreeid(NodeID id);

        void fatten(Bounds &bounds);

        void insertLeaf(NodeID id);

        void removeLeaf(NodeID id);

        void syncHeirarchy(NodeID id);

        NodeID balance(NodeID id);

        int getHeight(NodeID id) const;

        int getBalance(NodeID id) const;
    };
};



