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

Animator::Animator() : m_animation(nullptr), m_armature(nullptr), m_currentTime(0.0f), m_FinalBoneMatrices(100, Matrix4x4::identityMatrix), m_nextAnimation(nullptr), queueAnimation(nullptr), haltTime(0.0f), interTime(0.0f), bInterpolating(false)
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
    m_currentTime += m_animation->getTick() * Clock::deltaTime;
    m_currentTime = std::fmod(m_currentTime, m_animation->getDuration());

    auto transform = m_armature->getTransform();

    for_each(transform, Matrix4x4::identityMatrix);
    /*
    auto transform = m_armature->getTransform();

	if (m_animation) {
		m_currentTime = fmod(m_currentTime + m_animation->getTick() * Clock::deltaTime, m_animation->getDuration());

		if (bInterpolating) {
			interTime += m_animation->getTick() * Clock::deltaTime;
			calculateBoneTransition(transform, Matrix4x4::identityMatrix, m_animation, m_nextAnimation, haltTime, interTime, 0.5f);
			return;
		}
		else if (bInterpolating) {
			if (queueAnimation) {
				m_animation = m_nextAnimation;
				haltTime = 0.0f;
				m_nextAnimation = queueAnimation;
				queueAnimation = nullptr;
				m_currentTime = 0.0f;
				interTime = 0.0;
				return;
			}

			bInterpolating = false;
			m_animation = m_nextAnimation;
			m_currentTime = 0.0;
			interTime = 0.0;
		}

        for_each(transform, Matrix4x4::identityMatrix);
	}
    */
}

float easeInOut(float t) {
    return t * t * (3.0f - 2.0f * t);
}

void Animator::calculateBoneTransition(const Transform* transform, Matrix4x4 parent, Animation* prevAnimation, Animation* nextAnimation, float haltTime, float currentTime, float transitionTime)
{
    /*
    auto boneName = transform->getGameObject()->getName();

    auto prevKeyframe = prevAnimation->findKeyframe(boneName);
    auto nextKeyframe = nextAnimation->findKeyframe(boneName);
    
    auto it = m_bones.find(boneName);

    if (prevKeyframe != nullptr && nextKeyframe != nullptr && it != m_bones.end())
    {

		auto prevPos = prevKeyframe->getPositionFromTime(haltTime);
		auto prevRot = prevKeyframe->getRotationFromTime(haltTime);
		auto prevScl = prevKeyframe->getScaleFromTime(haltTime);

		auto nextPos = nextKeyframe->getPositionFromTime(0.0f);
		auto nextRot = nextKeyframe->getRotationFromTime(0.0f);
		auto nextScl = nextKeyframe->getScaleFromTime(0.0f);

        prevPos.time = 0.0f;
        prevRot.time = 0.0f;
        prevScl.time = 0.0f;

        nextPos.time = transitionTime;
        nextRot.time = transitionTime;
        nextScl.time = transitionTime;

        parent *= Math::CreateTRS(
                Keyframe::GetInterpolateScaling(m_currentTime, prevScl, nextScl),
                Keyframe::GetInterpolateRotation(m_currentTime, prevRot, nextRot),
                Keyframe::GetInterpolatePosition(m_currentTime, prevPos, nextPos)
                );

        m_FinalBoneMatrices[it->second->getBoneIndex()] = parent * it->second->getLocalMatrix();
    }

    for (auto child : transform->getChildren()) {
        for_each(child, parent);
    }
    */
}

void Animator::for_each(Transform *transform, Matrix4x4 parent)
{
    auto bonecount = m_bones.size();

    std::vector<Matrix4x4> parentList(bonecount, Matrix4x4::identityMatrix);
    for (std::size_t i={};i<bonecount;i++)
    {
        auto parentIndex = m_bones[i].getParentIndex();

        parentList[i] = parentList[parentIndex] * m_animation->getKeyFrame(i)->getLocalMatrix(m_currentTime);
    }

    for (std::size_t i=0;i<bonecount;i++)
    {
        m_FinalBoneMatrices[i] = parentList[i] * m_bones[i].getLocalMatrix();
    }

    /*
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
    */
}

void Animator::setArmature(GameObject *armature)
{
    m_armature = armature;
}

GameObject *Animator::getArmature() const
{
    return m_armature;
}

void Animator::play(bool repeat, Animation *animation)
{
    m_animation = animation;
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







