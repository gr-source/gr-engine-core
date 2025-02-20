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
    Material();
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
    grr::gShader *m_shader;

    std::vector<std::tuple<int, const void *>> m_uniformList;
    
    std::unordered_map<std::string, void *> m_uniformMap;
    
    std::array<grr::gTexture *, 10> m_texture;

    bool setupTexture(int id, grr::gTexture *texture);

    void *internal_set_value(const char *name, grm::u8 count, const void *data);
};

template <typename T>
inline T *Material::getValue(const std::string &name)
{
    return reinterpret_cast<T *>(m_uniformMap[name]);
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
