#include "gr-base/system/camerasystem.hpp"

// #include "gr-base/core/rendertarget.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

void CameraSystem::Update()
{
    auto world = scene->world;

    world->Query<TransformWorld, Camera, CameraData>([&](
                size_t count,
                const Entity* entities,
                const TransformWorld* transforms,
                const Camera* cameras,
                CameraData* datas)
    {
        for (size_t i=0;i<count;i++)
        {
            const TransformWorld& transform = transforms[i];
            const Camera& camera = cameras[i];
            CameraData& data = datas[i];

            float aspectRatio =
                camera.viewport.w / camera.viewport.h;

            data.projection =
                Math::perspective(
                    Math::rad(camera.fov),
                    aspectRatio,
                    camera.nearPlane,
                    camera.farPlane
                );

            data.view = Math::inverse(transform.world);
        }
    });
}

Matrix4x4 CameraSystem::CreateViewMatrix(const TransformWorld* transform) const
{
    return {};
    /*
    auto invRotation =
        Math::normalize(Math::inverse(transform->rotation));

    auto invPosition =
        -(invRotation * transform->position);

    return
        Math::rotate<Matrix4x4>(invRotation) *
        Math::translate<Matrix4x4>(invPosition);
    */
}

