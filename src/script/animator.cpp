#include "gr-base/script/animator.hpp"

#include "gr-base/core/animation/animation.hpp"
#include "gr-base/core/animation/bone.hpp"

// #include "core/timer.hpp"

#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"

// #include "renderer.h"

void Animator::update() {
    /*
    if (!isReady()) { return; }
    
    m_currentTime += m_animation->getTick() * TimeLast::deltaTime;
    m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

    auto transform = m_armature->getTransform();

    for_each(transform, Matrix4x4::identityMatrix);
    */
}

bool Animator::isReady() const {
    return m_armature != nullptr && m_animation != nullptr && !m_bones.empty();
}

GameObject *Animator::getArmature() const {
    return m_armature;
}

void Animator::setArmature(GameObject *armature) {
    m_armature = armature;
}

Animation *Animator::getAnimation() const {
    return m_animation;
}

void Animator::setAnimation(Animation *animation) {
    m_animation = animation;
}

const std::vector<Matrix4x4> &Animator::getBoneData() const {
    return m_FinalBoneMatrices;
}

void Animator::for_each(Transform *transform, Matrix4x4 parent) {
    auto boneName = transform->getGameObject()->getName();

    auto keyframe = m_animation->findKeyframe(boneName);
    auto it = m_bones.find(boneName);
    if (keyframe && it != m_bones.end()) {
        parent *= keyframe->getLocalMatrix(m_currentTime);

        m_FinalBoneMatrices[it->second->getBoneIndex()] = parent * it->second->getLocalMatrix();
    }

    for (auto child : transform->getChildren()) {
        for_each(child, parent);
    }
}
/*

void Animator::hierarchy(Transform *transform, const Matrix4x4 &parentTransform) {
    auto gameObject = transform->getGameObject();

    Matrix4x4 result = parentTransform;

    if (!gameObject) {
        return;
    }
    
    auto name = gameObject->getName();

    auto keyframe = m_animation->findKeyframe(name);
    auto bone = findBone(name);

    if (keyframe && bone) {
        Matrix4x4 offset = bone->getLocalMatrix();
        result *= keyframe->getLocalMatrix(m_currentTime);

        m_finalBoneMatrices[bone->getBoneIndex()] = result * offset;
    }

    for (const auto &child : transform->getChildren()) {
        hierarchy(child, result);
    }
}

*/

