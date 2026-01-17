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
    glDeleteBuffers(1, &pbo);
}

void Batching::insert(ComponentID transform)
{
    instances.push_back(transform);
}

void Batching::remove(ComponentID transform)
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
        glGenBuffers(1, &pbo);
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

    glBindBuffer(GL_ARRAY_BUFFER, pbo);
    for (int j=0;j<4;j++)
    {
        glEnableVertexAttribArray(6 + j);
    
        glVertexAttribPointer(6 + j, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4), (void*)(uintptr_t)(j * sizeof(Vector4)));

        glVertexAttribDivisor(6 + j, 1);
        /*
        grr::gVertexArray::SetAttrib(6 + j, 4, sizeof(Matrix4x4), reinterpret_cast<const void*>(j * sizeof(Vector4)));
        grr::gVertexArray::SetAttribDivisor(6 + j, 1);
        */
    }

    glBindVertexArray(0);
    
    std::cout << "[GPU] Batch criado com Mesh: " << meshID << " e Material: " << materialID << std::endl;
}

void Batching::render()
{
    bool isInstancing = instances.size() > 1;

    auto &material = AssetsManager::GetMaterial(materialID);
    auto shader = material.GetShader(isInstancing);

    glBindVertexArray(vao);

    bool enable[10] = {false};
    int textureID[10] = {0};

    shader->bind();
    material.flush(shader);

    shader->setUniform("u_texture", textureID);
    shader->setUniform("u_enable", enable);

    if (isInstancing)
    {
        static size_t lastSize = 0;
        if (lastSize != instances.size())
        {
            std::vector<Matrix4x4> models;
            for (auto& transformID : instances)
            {
                auto transform = ComponentManager::GetComponent<Transform>(transformID);

                models.push_back(transform->getGlobalMatrix());
            }

            glBindBuffer(GL_ARRAY_BUFFER, pbo);
            glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(Matrix4x4), models.data(), GL_STATIC_DRAW);

            lastSize = instances.size();
        }
        
        shader->flush();

        glDrawElementsInstanced(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr, lastSize);

    } else if (!instances.empty())
    {
        auto transform = ComponentManager::GetComponent<Transform>(instances[0]);

        auto model = transform->getGlobalMatrix();

        shader->setUniform("u_model", &model);
        shader->flush();

        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr);
    }
    
    glBindVertexArray(0);
}






