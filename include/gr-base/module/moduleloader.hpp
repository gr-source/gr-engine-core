#pragma once

#include <unordered_map>
#include <string>

class ModuleLoader
{
public:
    static void Initialize();

    static void Shutdown();

    inline static void Load(const std::string& name, const std::string& path)
    {
        return s_ModuleLoader->load(name, path);
    }

    inline static void Unload(const std::string& name)
    {
        return s_ModuleLoader->unload(name);
    }

    static bool CheckModuleHasInMemory(const char* name);

    template <typename R, typename... Args>
    inline static R (*GetFunction(const std::string& module, const std::string& name))(Args...)
    {
        auto ptr = s_ModuleLoader->findSymbol(module, name);
        if (ptr == nullptr)
            return nullptr;
        return reinterpret_cast<R (*)(Args...)>(ptr);
    }

private:
    static ModuleLoader* s_ModuleLoader;

    std::unordered_map<std::string, void*> m_moduleMap;

    ModuleLoader();
    ~ModuleLoader();

    void load(const std::string& name, const std::string& path);

    void unload(const std::string& name);

    void unload();

    void* getModule(const std::string& name);

    void* findSymbol(const std::string& module, const std::string& name);
};


