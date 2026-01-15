#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/script/transform.hpp"

#include <gr-render/gl.h>
#include <gr-render/gVertexArray.h>

#include <algorithm>

// public

Batching::Batching() : vao(0) {}

Batching::~Batching()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void Batching::insert(Transform *transform)
{
    instances.push_back(transform);
}

void Batching::remove(Transform *transform)
{
    instances.erase(std::remove(instances.begin(), instances.end(), transform), instances.end());
}

void Batching::build(MeshID meshID, MaterialID materialID)
{
    this->materialID = materialID;

    const Mesh& data = AssetsManager::GetMesh(meshID);
    this->vertexCount = data.getIndices().size();

    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.getVertices().size() * sizeof(Vertex3D), data.getVertices().data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.getIndices().size() * sizeof(uint32_t), data.getIndices().data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, position));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, texCoords));

    glBindVertexArray(0);
    
    std::cout << "[GPU] Batch criado com Mesh: " << meshID << " e Material: " << materialID << std::endl;
}

void Batching::render()
{
    auto &material = AssetsManager::GetMaterial(materialID);
    auto shader = material.getShader();

    glBindVertexArray(vao);

    bool enable[10] = {false};
    int textureID[10] = {0};

    shader->bind();
    material.flush();

    shader->setUniform("u_texture", textureID);
    shader->setUniform("u_enable", enable);

    for (auto transform : instances)
    {
        auto model = transform->getGlobalMatrix();

        shader->setUniform("u_model", &model);
        shader->flush();

        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
}






