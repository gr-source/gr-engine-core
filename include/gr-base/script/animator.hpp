#pragma once

#include "gr-base/core/animation/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/ecs/component.hpp"

class Animator : public Component {
public:
    Animator() = delete;

    void update();

    bool isReady() const;

    GameObject *getArmature() const;

    void setArmature(GameObject *armature);

    Animation *getAnimation() const;

    void setAnimation(Animation *animation);

    const std::vector<Matrix4x4> &getBoneData() const;

    std::unordered_map<std::string, std::shared_ptr<Bone>> m_bones;

private:
    std::vector<Matrix4x4> m_FinalBoneMatrices;

    GameObject *m_armature;

    Animation *m_animation;

    float m_currentTime;

    void for_each(Transform *transform, Matrix4x4 parent = Matrix4x4::identityMatrix);
};
