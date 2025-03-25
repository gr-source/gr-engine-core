#include "gr-base/script/animator.hpp"

#include "gr-base/core/animation/keyframe.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/clock.hpp"

#include "gr-base/core/animation/animation.hpp"
#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"
#include <cstddef>
#include <gr-math/gmath.hpp>
#include <gr-math/matrix4x4.hpp>
#include <gr-math/quaternion.hpp>
#include <gr-math/vector3.hpp>
#include <iterator>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

Animator::Animator() : m_animation(nullptr), m_currentTime(0.0f), m_FinalBoneMatrices(100, Matrix4x4::identityMatrix), m_nextAnimation(nullptr), m_nextTime(0.0f), blendFactor(0.0f)
{
}

void Animator::Init()
{
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
    auto boneMapCount = m_bones.size();
    if (!stream->write(&boneMapCount, sizeof(std::size_t)))
    {
        return false;
    }

    for (auto &bone : m_bones)
    {
        /*auto nsize = name.size();
        if (!stream->write(&nsize, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(name.c_str(), nsize * sizeof(char)))
        {
            return false;
        }
        */

        bone.serialize(stream);
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

    m_bones.resize(boneMapCount, {});

    for (std::size_t i=0;i<boneMapCount;i++)
    {

        /*
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
        */

        {
            /*m_bones.emplace_back(std::make_shared<Bone>());*/
        }

        m_bones[i].deserialize(stream);

        /*std::cout << m_bones[i]->getName() << std::endl;*/
    }
    return true;
}

void Animator::updateFrame()
{
    if (m_animation == nullptr || m_nextAnimation == nullptr)
    {
        return;
    }

    float a = 1.0f;
    float b = m_animation->getDuration() / m_nextAnimation->getDuration();
    const float animSpeedMultiplierUp = (1.0f - blendFactor) * a + b * blendFactor; // Lerp

    a = m_nextAnimation->getDuration() / m_animation->getDuration();
    b = 1.0f;
    const float animSpeedMultiplierDown = (1.0f - blendFactor) * a + b * blendFactor; // Lerp

    // Current time of each animation, "scaled" by the above speed multiplier variables
    static float currentTimeBase = 0.0f;
    currentTimeBase += m_animation->getTick() * Clock::deltaTime * animSpeedMultiplierUp;
    currentTimeBase = fmod(currentTimeBase, m_animation->getDuration());

    static float currentTimeLayered = 0.0f;
    currentTimeLayered += m_nextAnimation->getTick() * Clock::deltaTime * animSpeedMultiplierDown;
    currentTimeLayered = fmod(currentTimeLayered, m_nextAnimation->getDuration());

    updateBoneTransform(m_animation, currentTimeBase, m_nextAnimation, currentTimeLayered, blendFactor);
}

void Animator::updateBoneTransform(Animation *animation, float time)
{
    auto bonecount = m_bones.size();

    std::vector<Matrix4x4> parentList(bonecount, Matrix4x4::identityMatrix);
    for (std::size_t i={};i<bonecount;i++)
    {
        auto parentIndex = m_bones[i].getParentIndex();

        parentList[i] = parentList[parentIndex] * animation->getKeyFrame(i)->getLocalMatrix(time);
    }

    for (std::size_t i=0;i<bonecount;i++)
    {
        m_FinalBoneMatrices[i] = parentList[i] * m_bones[i].getLocalMatrix();
    }
}


void Animator::updateBoneTransform(Animation *prevAnimation, float prevTime, Animation *nextAnimation, float nextTime, float factor)
{
    auto bonecount = m_bones.size();

    std::vector<Matrix4x4> parentList(bonecount, Matrix4x4::identityMatrix);
    for (std::size_t i={};i<bonecount;i++)
    {
        auto parentIndex = m_bones[i].getParentIndex();

        auto prevScale      = prevAnimation->getKeyFrame(i)->getScaleFromTime(prevTime);
        auto prevRotate     = prevAnimation->getKeyFrame(i)->getRotationFromTime(prevTime);
        auto prevPosition   = prevAnimation->getKeyFrame(i)->getPositionFromTime(prevTime);

        auto nextScale      = nextAnimation->getKeyFrame(i)->getScaleFromTime(nextTime);
        auto nextRotate     = nextAnimation->getKeyFrame(i)->getRotationFromTime(nextTime);
        auto nextPosition   = nextAnimation->getKeyFrame(i)->getPositionFromTime(nextTime);

        Vector3 s = (1.0f - factor) * prevScale.value + nextScale.value * factor;
        /*Vector3 s = Math::lerp(prevScale.value, nextScale.value, factor);*/
        Quaternion r = Math::slerp(prevRotate.value, nextRotate.value, factor);
        /*Vector3 t = Math::lerp(prevPosition.value, nextPosition.value, factor);*/
        Vector3 t = (1.0f - factor) * prevPosition.value + nextPosition.value * factor;
        
        parentList[i] = parentList[parentIndex] * Math::CreateTRS(
                s, r, t
                );
    }

    for (std::size_t i=0;i<bonecount;i++)
    {
        m_FinalBoneMatrices[i] = parentList[i] * m_bones[i].getLocalMatrix();
    }
}

void Animator::play(bool repeat, Animation *animation)
{
    if (m_animation == nullptr)
    {
        m_animation = animation;
        return;
    }

    if (m_nextAnimation != animation)
        m_nextAnimation = animation;
	/*
    if (!m_animation)
    {
		m_animation = animation;
		return;
	}

	if (bInterpolating)
    {
		if (animation != m_nextAnimation)
			queueAnimation = animation;
	}
	else {
		if (animation != m_nextAnimation) {
			bInterpolating = true;
			haltTime = fmod(m_currentTime, m_animation->getDuration());
			m_nextAnimation = animation;
			m_currentTime = 0.0f;
			interTime = 0.0;
		}
	}
    */
}

Animation *Animator::getAnimation() const
{
    return m_animation;
}







