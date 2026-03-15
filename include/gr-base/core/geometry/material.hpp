#pragma once

#include <cstdint>
#include <string>
#include <array>

#include <gr-render/gRender.h>
#include <unordered_map>

class Stream;

typedef struct
{
    void *data;
    u32 id;
} Params;

class Material
{
public:
    Material();
    ~Material();

    bool bEnableInstancing;
    
    void setTexture(int idx, grr::gTexture *texture);

    grr::gTexture *getTexture(int idx) const;

    std::array<grr::gTexture *, 10> getTextures() const;

    grr::gShader *GetShader(bool instancing) const;

    void SetShader(grr::gShader* shader, bool instancing);

    bool supportsInstancing() const;

    void set_scaling(const Vector2 &value);

    Vector2 &get_scalling();

    void set_offset(const Vector2 &value);

    Vector2 &get_offset();

    void serialize(Stream *stream) const;

    void deserialize(Stream *stream);

    template <typename T>
    T *getValue(const std::string &name);

    template <typename T>
    T *setValue(const std::string &name, const T &value);

    template <typename T>
    T *setValue(const std::string &name, u8 count, const T &value);

    void flush(grr::gShader* shader) const;

private:
    std::unordered_map<std::string, std::tuple<void *, std::size_t>> m_field_data;

    std::vector<std::tuple<void *, int>> m_field_list;

    std::array<grr::gTexture *, 10> m_textures;

    Vector2 m_scalling;

    Vector2 m_offset;

    grr::gShader *m_shader;
    grr::gShader *m_shaderInstancing;

    bool setupTexture(int id, grr::gTexture *texture);

    void *internal_set_value(const char *name, u8 count, const void *data);
};

template <typename T>
inline T *Material::getValue(const std::string &name)
{
    auto it = m_field_data.find(name);
    if (it == m_field_data.end())
    {
        return nullptr;
    }
    return reinterpret_cast<T *>(std::get<0>(it->second));
}

template <typename T>
inline T *Material::setValue(const std::string &name, const T &value)
{
    return setValue(name, 1, value);
}

template <typename T>
inline T *Material::setValue(const std::string &name, u8 count, const T &value)
{
    return reinterpret_cast<T *>(internal_set_value(name.c_str(), count, &value));
}
