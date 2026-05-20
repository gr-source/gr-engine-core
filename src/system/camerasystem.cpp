#include "gr-base/system/camerasystem.hpp"

#include "gr-base/core/rendertarget.hpp"

#include "gr-base/ecs/view.hpp"
#include "gr-base/ecs/ecs.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

void CameraSystem::Update(ECS* world)
{
    for (auto&& [e, transform, camera, data] : world->Query<Transform, Camera, CameraData>())
    {
        float aspectRatio = static_cast<float>(camera->rendertarget->GetWidth()) / static_cast<float>(camera->rendertarget->GetHeight());

        data->projection =
            Math::perspective(
                Math::rad(camera->fov),
                aspectRatio,
                camera->nearPlane,
                camera->farPlane
            );

        data->forward =
            transform->rotation * Vector3::forward;

        data->right =
            transform->rotation * Vector3::right;

        data->up =
            transform->rotation * Vector3::up;

        data->view = Math::lookAt(transform->position, transform->position + data->forward, data->up);

        // data->view = Math::Inverse(transform->worldMatrix);

        data->viewProjection =
            data->projection *
            data->view;
    }
}
