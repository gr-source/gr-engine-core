#pragma once

#include "gr-base/core/animation/animation.hpp"
#include "gr-base/core/animation/default.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/script/default.hpp"

#include "gr-base/ecs/component.hpp"
#include "gr-base/script/transform.hpp"
#include <gr-math/matrix4x4.hpp>

class Animator : public Component {
public:
    Animator();

    void Init() override;

    void Update() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    void setArmature(GameObject *armature);

    GameObject *getArmature() const;

    void play(bool repeate, Animation *animation);

    Animation *getAnimation() const;

    std::unordered_map<std::string, std::shared_ptr<Bone>> m_bones;

    std::vector<Matrix4x4> m_FinalBoneMatrices;

private:
    GameObject *m_armature;

    Animation *m_animation;

    Animation *m_nextAnimation;

    Animation *queueAnimation;

    float m_currentTime;

    float haltTime;

    float interTime;

    bool bInterpolating;

    void updateFrame();

	void calculateBoneTransition(const Transform* transform, Matrix4x4 parentTransform, Animation* prevAnimation, Animation* nextAnimation, float haltTime, float currentTime, float transitionTime);

    void for_each(Transform *transform, Matrix4x4 parent);
};

REGISTER_COMPONENT(Animator, true);


