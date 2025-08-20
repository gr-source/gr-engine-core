#pragma once

#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/geometry/batchingmanager.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/entity.hpp"

class BaseRender : public Component
{
    CLASS_FUNCTIONS
public:
    BaseRender();

    void onInit() override;

    void onRelease() override;

    void onEnable() override;

    void onDisable() override;

    bool onSerialize(Stream *stream) override;

    bool onDeserialize(Stream *stream) override;

    virtual void build(std::vector<Mesh> *meshlist) {}

    template <typename ...Args>
    Material *createMaterial(Args &&...args);

    Material *addMaterial(Material *material);

    void delMaterial(Material *material);

    bool hasMaterial(const Material *material) const;

    std::vector<Material *> getMaterial() const;

    const std::vector<Material *> &getMaterialRef() const;

private:
    std::vector<Material *> m_material;

    // BatchID m_batchID;

    friend class SkinnedMeshRender;
    friend class MeshRender;
};

#include "gr-base/core/assetsManager.hpp"

template <typename ...Args>
inline Material *BaseRender::createMaterial(Args &&...args)
{
    return addMaterial(static_cast<Material *>(AssetsManager::CreateMaterial(std::forward<Args>(args)...)));
}

REGISTER_COMPONENT(BaseRender)




