#include "gr-base/script/animator.hpp"

#include "gr-base/core/animation/keyframe.hpp"
#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/clock.hpp"

#include "gr-base/core/animation/animation.hpp"
#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include <cstddef>

Animator::Animator() : m_animation(nullptr), m_currentTime(0.0f), m_FinalBoneMatrices(100, Matrix4x4::identityMatrix), m_nextAnimation(nullptr), m_nextTime(0.0f), blendFactor(0.5f), interpolating(false), queueAnimation(nullptr), haltTime(0.0f), interTime(0.0f), m_skinnedMeshRender(nullptr)
{
}

void Animator::Init()
{
    bCanUseUpdate = true;
}

void Animator::Start()
{
    if (!HasComponent<SkinnedMeshRender>())
    {
        return;
    }

    m_skinnedMeshRender = GetComponent<SkinnedMeshRender>();

    bCanUseUpdate = true;
}

void Animator::Update()
{
    if (m_animation == nullptr)
    {
        return;
    }

    updateFrame();
}

bool Animator::Serialize(Stream *stream)
{
    if (m_animation == nullptr)
    {
        return false;
    }
    
    auto name = AssetsManager::GetAnimationName(m_animation);

    auto nlen = name.size();
    if (!stream->write(&nlen, sizeof(std::size_t)))
    {
        return false;
    }

    if (!stream->write(name.c_str(), nlen * sizeof(char)))
    {
        return false;
    }

    return true;
}

bool Animator::Deserialize(Stream *stream)
{
    std::size_t nlen = {};
    if (!stream->read(reinterpret_cast<char *>(&nlen), sizeof(std::size_t)))
    {
        return false;
    }

    std::string name(nlen, '\0');
    if (!stream->read(name.data(), nlen * sizeof(char)))
    {
        return false;
    }

    auto animation = AssetsManager::FindAnimation(name);
    if (animation != nullptr)
    {
        m_animation = animation;
        return true;
    }
    
    AssetsManager::LoadAnimation(name, &Animator::play, this, false);
    return true;
}

void Animator::play(bool repeat, Animation *animation)
{
    if (m_animation == nullptr)
    {
        m_animation = animation;
        return;
    }
    
    if (animation != m_animation && m_nextAnimation != animation)
    {
        m_nextAnimation = animation;

        m_nextTime = 0.0f;
    }
}

Animation *Animator::getAnimation() const
{
    return m_animation;
}

void Animator::updateFrame()
{
    if (m_nextAnimation != nullptr)
    {
        float a = 1.0f;
        float b = m_animation->getDuration() / m_nextAnimation->getDuration();
        const float animSpeedMultiplierUp = Math::lerp(a, b, blendFactor);

        a = m_nextAnimation->getDuration() / m_animation->getDuration();
        b = 1.0f;
        const float animSpeedMultiplierDown = Math::lerp(a, b, blendFactor);

        m_currentTime += m_animation->getTick() * Clock::deltaTime * animSpeedMultiplierUp;

        m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

        m_nextTime += m_nextAnimation->getTick() * Clock::deltaTime * animSpeedMultiplierDown;
        float blendFactorf = ((m_nextTime * 5.0f)/ m_nextAnimation->getDuration());

        m_nextTime = std::fmod(m_nextTime, m_nextAnimation->getDuration());

        updateBoneTransform(m_animation, m_currentTime, m_nextAnimation, m_nextTime, blendFactorf);
        if (blendFactorf >= 1.0f)
        {
            m_animation = m_nextAnimation;
            m_nextAnimation = nullptr;
            m_currentTime = m_nextTime;
            m_nextTime = 0.0f;
        }
        return;
    }

    m_currentTime += m_animation->getTick() * Clock::deltaTime;
    m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

    updateBoneTransform(m_animation, m_currentTime);
}

void Animator::updateBoneTransform(Animation *animation, float time)
{
    auto bonecount = m_skinnedMeshRender->getBoneCount();

    std::vector<Matrix4x4> parentList(bonecount, Matrix4x4::identityMatrix);
    for (std::size_t i={};i<bonecount;i++)
    {
        auto parentIndex = m_skinnedMeshRender->getBone(i)->getParentIndex();

        parentList[i] = parentList[parentIndex] * animation->getKeyFrame(i)->getLocalMatrix(time);
    }

    for (std::size_t i=0;i<bonecount;i++)
    {
        m_FinalBoneMatrices[i] = parentList[i] * m_skinnedMeshRender->getBone(i)->getLocalMatrix();
    }
}


void Animator::updateBoneTransform(Animation *prevAnimation, float prevTime, Animation *nextAnimation, float nextTime, float factor)
{
    auto bonecount = m_skinnedMeshRender->getBoneCount();

    std::vector<Matrix4x4> parentList(bonecount, Matrix4x4::identityMatrix);
    for (std::size_t i={};i<bonecount;i++)
    {
        auto parentIndex = m_skinnedMeshRender->getBone(i)->getParentIndex();

        auto prevScale      = prevAnimation->getKeyFrame(i)->getScaleFromTime(prevTime);
        auto prevRotate     = prevAnimation->getKeyFrame(i)->getRotationFromTime(prevTime);
        auto prevPosition   = prevAnimation->getKeyFrame(i)->getPositionFromTime(prevTime);

        auto nextScale      = nextAnimation->getKeyFrame(i)->getScaleFromTime(nextTime);
        auto nextRotate     = nextAnimation->getKeyFrame(i)->getRotationFromTime(nextTime);
        auto nextPosition   = nextAnimation->getKeyFrame(i)->getPositionFromTime(nextTime);

        Vector3 s       =   Math::mix(nextScale.value, prevScale.value, factor);
        Quaternion r    =   Math::normalize(Math::slerp(prevRotate.value, nextRotate.value, factor));
        Vector3 t       =   Math::mix(nextPosition.value, prevPosition.value, factor);
        
        parentList[i] = parentList[parentIndex] * Math::CreateTRS(
                t, r, s
                );
    }

    for (std::size_t i=0;i<bonecount;i++)
    {
        m_FinalBoneMatrices[i] = parentList[i] * m_skinnedMeshRender->getBone(i)->getLocalMatrix();
    }
}



