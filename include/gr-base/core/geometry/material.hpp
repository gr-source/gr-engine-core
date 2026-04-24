#pragma once

#include <cstdint>
#include <string>
#include <array>

#include <gr-render/gRender.h>

class Material
{
public:
    Material();
    ~Material();
    
    void setTexture(int idx, gr::gTexture *texture);

    gr::gTexture *getTexture(int idx) const;

    std::array<gr::gTexture *, 10> getTextures() const;

    inline gr::Shader *GetShader() const
    {
        return m_shader;
    }

    inline void SetShader(gr::Shader* shader)
    {
        m_shader = shader;
    }

    void SetEnableInstancing();

    void SetDisableInstancing();

    bool supportsInstancing() const;

    template <typename T>
    T *Get(const std::string &name);

    template <typename T>
    void Set(const std::string &name, const T &value);

    void Set(const char *name, u8 count, const void *data);

    void flush(gr::Shader* shader) const;

private:
    std::array<gr::gTexture *, 10> m_textures;

    std::vector<uint8_t> m_buffer;

    gr::Shader *m_shader;

    bool bEnableInstancing;

    bool setupTexture(int id, gr::gTexture *texture);

    void* Get(const char* name);
};

template <typename T>
inline T *Material::Get(const std::string &name)
{
    return reinterpret_cast<T*>(Get(name.c_str()));
}

template <typename T>
inline void Material::Set(const std::string &name, const T &value)
{
    Set(name.c_str(), 1, (const void*)&value);
}


