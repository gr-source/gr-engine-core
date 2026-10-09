#pragma once

#include <unordered_map>
#include <string>

class ModuleLoader
{
public:
    static void Initialize();

    static void Shutdown();

    static void Load(const std::string& name, const std::string& module);

    static void Unload(const std::string& name);

    static void UnloadAll();

    static bool CheckModuleHasInMemory(const char* name);

    template <typename R, typename... Args>
    static R (*GetFunction(const std::string& module, const std::string& name))(Args...);

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

template <typename R, typename... Args>
inline R (*ModuleLoader::GetFunction(const std::string& module, const std::string& name))(Args...)
{
    auto ptr = s_ModuleLoader->findSymbol(module, name);
    if (ptr == nullptr)
        return nullptr;
    return reinterpret_cast<R (*)(Args...)>(ptr);
}

