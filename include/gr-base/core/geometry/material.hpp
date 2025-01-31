#pragma once

#include <string>

#include <gr-render/gRender.h>

class Stream;

class Material {
public:
    Material(const std::string& name = "Material", const Color& albedo = Color::white, float metallic = 0.0f, float roughness = 0.0f, float ao = 0.5f);

    std::string getName() const;

    void setName(const std::string &name);

    Color getAlbedo() const;

    void setAlbedo(const Color &color);

    float getMetallic() const;

    void setMetallic(float value);
    
    float getRoughness() const;

    void setRoughness(float value);

    float getAmbientOcclusion() const;

    void setAmbientOcclusion(float value);

    void setTexture(int idx, grr::gTexture *texture);

    grr::gTexture *getTexture(int idx) const;

    void serialize(Stream *stream) const;

    void deserialize(Stream *stream);

private:
    std::string m_name;

    Color m_albedo;

    float m_metallic;

    float m_roughness;

    float m_ao;

    std::vector<grr::gTexture *> m_texture;

    bool setupTexture(int id, grr::gTexture *texture);
};

