#include "gr-base/script/animator.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/clock.hpp"

#include "gr-base/core/animation/animation.hpp"
#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>


Animator::Animator() : m_animation(nullptr), m_armature(nullptr), m_currentTime(0.0f), m_FinalBoneMatrices(100, Matrix4x4::identityMatrix)
{
}

void Animator::Init()
{
    bCanUseUpdate = true;
}

void Animator::Update()
{
    if (m_animation == nullptr || m_armature == nullptr)
    {
        return;
    }

    updateFrame();
}

bool Animator::Serialize(Stream *stream)
{
    auto boneMapCount = m_bones.size();
    if (!stream->write(&boneMapCount, sizeof(std::size_t)))
    {
        return false;
    }

    for (auto &&[name, bone] : m_bones)
    {
        auto nsize = name.size();
        if (!stream->write(&nsize, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(name.c_str(), nsize * sizeof(char)))
        {
            return false;
        }

        bone->serialize(stream);
    }
    return true;
}

bool Animator::Deserialize(Stream *stream)
{
    std::size_t boneMapCount = 0;
    if (!stream->read(reinterpret_cast<char *>(&boneMapCount), sizeof(std::size_t)))
    {
        return false;
    }

    for (std::size_t i=0;i<boneMapCount;i++)
    {
        std::size_t nsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
        {
            return false;
        }
        
        std::string name(nsize, '\0');
        if (!stream->read(name.data(), nsize * sizeof(char)))
        {
            return false;
        }

        {
            m_bones.emplace(name, std::make_shared<Bone>());
        }

        m_bones[name]->deserialize(stream);
    }
    return true;
}

void Animator::updateFrame()
{
    m_currentTime += m_animation->getTick() * Clock::deltaTime;
    m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

    auto transform = m_armature->getTransform();

    for_each(transform, Matrix4x4::identityMatrix);
}

void Animator::for_each(Transform *transform, Matrix4x4 parent) {
    auto boneName = transform->getGameObject()->getName();

    auto keyframe = m_animation->findKeyframe(boneName);
    
    auto it = m_bones.find(boneName);

    if (keyframe != nullptr && it != m_bones.end()) {
        parent *= keyframe->getLocalMatrix(m_currentTime);

        m_FinalBoneMatrices[it->second->getBoneIndex()] = parent * it->second->getLocalMatrix();
    }

    for (auto child : transform->getChildren()) {
        for_each(child, parent);
    }
}

void Animator::setArmature(GameObject *armature)
{
    m_armature = armature;
}

GameObject *Animator::getArmature() const
{
    return m_armature;
}

void Animator::setAnimation(Animation *animation)
{
    m_animation = animation;
}

Animation *Animator::getAnimation() const
{
    return m_animation;
}







