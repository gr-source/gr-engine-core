#pragma once

#include "fbxMaterial.hpp"
#include "fbxVertex.hpp"
#include "fbxBone.hpp"

#include <vector>

class FbxMesh {
public:
    FbxMesh(const std::string &name = "default");

    const std::string &getName() const;
    
    template <typename ...Args>
    FbxVertex &addVertice(Args &&...args);

    FbxVertex &getVertice(int idx);

    const std::vector<FbxVertex> &getVertices() const;

    void addIndice(int idx);
    
    void addTriangle(int ia, int ib, int ic);

    const std::vector<int> &getIndices() const;

    grm::size addMaterial(int idx);

    const std::vector<int> &getMaterials() const;

    template <typename ...Args>
    FbxBone &addBone(Args &&...args);

    FbxBone *findBone(const std::string &name);

    std::vector<FbxBone> getBones() const;

    bool hasBone() const;
    
private:
    std::string m_name;

    std::vector<int> m_materials;

    std::vector<FbxVertex> m_vertices;

    std::vector<int> m_indices;

    std::vector<FbxBone> m_bones;
};

template <typename... Args>
inline FbxVertex &FbxMesh::addVertice(Args &&...args) {
    return m_vertices.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxBone &FbxMesh::addBone(Args &&...args) {
    return m_bones.emplace_back(std::forward<Args>(args)...);
}
