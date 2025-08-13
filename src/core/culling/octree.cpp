#include "gr-base/core/culling/octree.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/script/transform.hpp"
#include <gr-math/vector3.hpp>

#include <algorithm>

Octree::Octree(const Vector3 &position, float radius, std::size_t capacity) : m_position(position), m_radius(radius), m_capacity(capacity), m_divided(false)
{}

bool Octree::insert(Transform *transform)
{
    if (!contains(transform->getPosition()))
    {
        return false;
    }

    if (m_data.size() < m_capacity)
    {
        m_pointmap.emplace(transform, transform->getPosition());
        m_data.emplace_back(transform);
        return true;
    } else
    {
        if (!m_divided)
            subdivide();

        for (auto &node : m_children)
        {
            if (node->insert(transform))
            {
                return true;
            }
        }
    }

    return false;
}

bool Octree::remove(Transform *transform)
{
    if (!contains(m_pointmap[transform]))
    {
        return false;
    }

    m_pointmap.erase(transform);
    m_data.erase(std::remove(m_data.begin(), m_data.end(), transform), m_data.end());

    if (m_divided)
    {
        int i = 0;
        for (auto &node : m_children)
        {
            if (node->remove(transform))
            {
                return true;
            }

            if (node->m_data.empty())
                i++;
        }
        if (i == 8)
        {
            for (int x=0;x<8;x++)
                m_children[x] = nullptr;

            m_divided = false;
        }
    }
    return false;
}

void Octree::render()
{
    drawWireCube(m_position, m_radius);

    if (m_divided)
    {
        for (auto &child : m_children)
        {
            if (child)
                child->render();
        }
    }
}

// private
bool Octree::contains(const Vector3 &point)
{
    return (point.x >= m_position.x - m_radius && point.x <= m_position.x + m_radius &&
            point.y >= m_position.y - m_radius && point.y <= m_position.y + m_radius &&
            point.z >= m_position.z - m_radius && point.z <= m_position.z + m_radius);
}

void Octree::subdivide()
{
    Vector3 center = m_position;
    float w = m_radius / 2.0f;
    float h = m_radius / 2.0f;
    float d = m_radius / 2.0f;

    Vector3 tfr{center.x + w, center.y + h, center.z - d}; // Top Front Right
    Vector3 tfl{center.x - w, center.y + h, center.z - d}; // Top Front Left
    Vector3 tbr{center.x + w, center.y + h, center.z + d}; // Top Back Right
    Vector3 tbl{center.x - w, center.y + h, center.z + d}; // Top Back Left

    Vector3 bfr{center.x + w, center.y - h, center.z - d}; // Bottom Front Right
    Vector3 bfl{center.x - w, center.y - h, center.z - d}; // Bottom Front Left
    Vector3 bbr{center.x + w, center.y - h, center.z + d}; // Bottom Back Right
    Vector3 bbl{center.x - w, center.y - h, center.z + d}; // Bottom Back Left

    m_children[0] = std::make_unique<Octree>(tfr, m_radius / 2, m_capacity);
    m_children[1] = std::make_unique<Octree>(tfl, m_radius / 2, m_capacity);
    m_children[2] = std::make_unique<Octree>(tbr, m_radius / 2, m_capacity);
    m_children[3] = std::make_unique<Octree>(tbl, m_radius / 2, m_capacity);
    m_children[4] = std::make_unique<Octree>(bfr, m_radius / 2, m_capacity);
    m_children[5] = std::make_unique<Octree>(bfl, m_radius / 2, m_capacity);
    m_children[6] = std::make_unique<Octree>(bbr, m_radius / 2, m_capacity);
    m_children[7] = std::make_unique<Octree>(bbl, m_radius / 2, m_capacity);

    m_divided = true;
}

void Octree::drawWireCube(const Vector3 &center, float size)
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



