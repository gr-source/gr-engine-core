#pragma once

#include <string>

#include <gr-render/gRender.h>

class Stream;

typedef struct
{
    void *data;
    grm::u32 id;
} Params;

class Material {
public:
    Material(const Color& albedo = Color::white, float metallic = 0.0f, float roughness = 0.0f, float ao = 0.5f);

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

    std::array<grr::gTexture *, 10> getTextures() const;

    grr::gShader *getShader() const;

    void setShader(grr::gShader *shader);

    void serialize(Stream *stream) const;

    void deserialize(Stream *stream);

    template <typename T>
    void setValue(const std::string &name, const T &value);

    template <typename T>
    void setValue(const std::string &name, grm::u8 count, const T &value);

    void flush() const;

private:
    grr::gShader *m_shader;

    std::array<Params, 100> m_paramsList;
    
    Color m_albedo;

    float m_metallic;

    float m_roughness;

    float m_ao;

    std::array<grr::gTexture *, 10> m_texture;

    bool setupTexture(int id, grr::gTexture *texture);

    void updateUniform();

    void internal_set_value(const char *name, grm::u8 count, const void *data);
};

template <typename T>
inline void Material::setValue(const std::string &name, const T &value)
{
    return setValue(name, 1, value);
}

template <typename T>
inline void Material::setValue(const std::string &name, grm::u8 count, const T &value)
{
    return internal_set_value(name.c_str(), count, &value);
}
