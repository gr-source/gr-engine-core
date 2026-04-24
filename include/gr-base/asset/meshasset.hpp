#pragma once


#include "gr-base/asset/asset.hpp"
#include "gr-base/core/geometry/vertex3D.hpp"

#include "gr-base/core/file/default.hpp"

#include <vector>

class SubMesh
{
public:
    std::vector<uint32_t> indices;

    std::string materialName;
};

class MeshAsset : public Asset
{
public:
    std::vector<Vertex3D> vertices;

    std::vector<SubMesh> submeshes;

    inline void SetName(const std::string& name)
    {
        m_name = name;
    }

    inline const std::string& GetName() const
    {
        return m_name;
    }

    bool serialize(Stream* stream) const override;

    bool deserialize(Stream* stream) override;

private:
    std::string m_name;
};


