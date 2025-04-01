#pragma once

#include "gr-base/ecs/component.hpp"

#include "gr-base/script/baserender.hpp"

#include "gr-base/core/animation/bone.hpp"

class SkinnedMeshRender : public BaseRender
{
    CLASS_FUNCTIONS
public:
    SkinnedMeshRender();

    void Init() override;

    void Destroy() override;

    void Enable() override;

    void Disable() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    void build() override;

    void setBones(const std::vector<Bone> &bones);

    Bone *getBone(int index);

    std::size_t getBoneCount() const;

private:
    std::vector<Bone> m_bones;
};

REGISTER_COMPONENT(SkinnedMeshRender, false)

