#pragma once

#include "gr-base/core/animation/animation.hpp"
#include "gr-base/core/animation/bone.hpp"
#include "gr-base/core/animation/default.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/ecs/entity.hpp"
#include <cstddef>

class Animator : public Component
{
    CLASS_FUNCTIONS
public:
    Animator();

    void Init() override;

    void Start() override;

    void Update() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    void play(bool repeate, Animation *animation);

    Animation *getAnimation() const;

    std::vector<Matrix4x4> m_FinalBoneMatrices;

    float blendFactor;

private:
    SkinnedMeshRender *m_skinnedMeshRender;

    Animation *m_animation;

    Animation *m_nextAnimation;

    Animation *queueAnimation;

    float m_currentTime;

    float m_nextTime;

    float haltTime;

    float interTime;

    bool interpolating;

    void updateFrame();

    void updateBoneTransform(Animation *animation, float time);

    void updateBoneTransform(Animation *prevAnimation, float prevTime, Animation *nextAnimation, float nextTime, float factor);
};

REGISTER_COMPONENT(Animator);


