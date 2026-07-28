#pragma once
/*

#include "gr-base/script/default.hpp"

#include <gr-math/gmath.hpp>

#include <gr-math/vector3.hpp>

#include <unordered_map>
#include <vector>
#include <memory>
#include <array>

class Octree
{
public:
    Octree(const Vector3 &position, float radius, std::size_t capacity);

    bool insert(Transform *transform);
    
    bool remove(Transform *transform);

    void render();

private:
    std::array<std::unique_ptr<Octree>, 8> m_children;

    std::unordered_map<Transform *, Vector3> m_pointmap;

    std::vector<Transform *> m_data;

    std::size_t m_capacity;

    Vector3 m_position;

    float m_radius;

    bool m_divided;

    bool contains(const Vector3 &point);

    void subdivide();

    void drawWireCube(const Vector3 &position, float size);
};

*/
