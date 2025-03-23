#pragma once

#include "gr-base/core/animation/default.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/script/default.hpp"

#include "gr-base/ecs/component.hpp"

class Animator : public Component {
public:
    Animator();

    void Init() override;

    void Update() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    void setArmature(GameObject *armature);

    GameObject *getArmature() const;

    void setAnimation(Animation *animation);

    Animation *getAnimation() const;

    std::unordered_map<std::string, std::shared_ptr<Bone>> m_bones;

    std::vector<Matrix4x4> m_FinalBoneMatrices;

private:
    GameObject *m_armature;

    Animation *m_animation;

    float m_currentTime;

    void updateFrame();

    void for_each(Transform *transform, Matrix4x4 parent);
};

REGISTER_COMPONENT(Animator, true);


