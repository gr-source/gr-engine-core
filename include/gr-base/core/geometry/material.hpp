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
    grm::u32 id;
} Params;

class Material
{
public:
    Material() = delete;
    Material(grr::gShader *shader);
    ~Material();
    
    void setTexture(int idx, grr::gTexture *texture);

    grr::gTexture *getTexture(int idx) const;

    std::array<grr::gTexture *, 10> getTextures() const;

    grr::gShader *getShader() const;

    void setShader(grr::gShader *shader);

    void serialize(Stream *stream) const;

    void deserialize(Stream *stream);

    template <typename T>
    T *getValue(const std::string &name);

    template <typename T>
    T *setValue(const std::string &name, const T &value);

    template <typename T>
    T *setValue(const std::string &name, grm::u8 count, const T &value);

    void flush() const;

private:
    std::unordered_map<std::string, std::tuple<void *, std::size_t>> m_field_data;


    std::vector<std::tuple<void *, int>> m_field_list;

    /*
    std::vector<std::tuple<int, const void *>> m_uniformList;
    
    std::unordered_map<std::string, void *> m_uniformMap;
    */
    
    std::array<grr::gTexture *, 10> m_textures;

    grr::gShader *m_shader;

    bool setupTexture(int id, grr::gTexture *texture);

    void *internal_set_value(const char *name, grm::u8 count, const void *data);
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
inline T *Material::setValue(const std::string &name, grm::u8 count, const T &value)
{
    return reinterpret_cast<T *>(internal_set_value(name.c_str(), count, &value));
}
