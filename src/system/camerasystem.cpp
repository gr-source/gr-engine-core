#include "gr-base/system/camerasystem.hpp"

#include "gr-base/core/rendertarget.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

void CameraSystem::Update(Scene* scene)
{
    auto world = scene->GetWorldDynamic();

    world->Query<Transform, Camera, CameraData>([](
                size_t count,
                const Entity* entities,
                Transform* transforms,
                Camera* cameras,
                CameraData* datas)
    {
        for (size_t i=0;i<count;i++)
        {
            Transform& transform = transforms[i];
            CameraData& data = datas[i];
            Camera& camera = cameras[i];

            float aspectRatio = static_cast<float>(camera.rendertarget->GetWidth()) / static_cast<float>(camera.rendertarget->GetHeight());

            data.projection =
                Math::perspective(
                    Math::rad(camera.fov),
                    aspectRatio,
                    camera.nearPlane,
                    camera.farPlane
                );

            data.forward =
                transform.rotation * Vector3::forward;

            data.right =
                transform.rotation * Vector3::right;

            data.up =
                transform.rotation * Vector3::up;

            data.view = Math::lookAt(transform.position, transform.position + data.forward, data.up);

            // data.view = Math::Inverse(transform.worldMatrix);

            data.viewProjection =
                data.projection *
                data.view;
        }
    });
}
