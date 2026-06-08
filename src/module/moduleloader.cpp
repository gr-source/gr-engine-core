#include "gr-base/module/moduleloader.hpp"

#include "gr-base/core/logger.hpp"

#include <stdexcept>

#include <string.h>
#include <dlfcn.h>

ModuleLoader* ModuleLoader::s_ModuleLoader = nullptr;

void ModuleLoader::Initialize()
{
    if (s_ModuleLoader == nullptr)
        s_ModuleLoader = new ModuleLoader();
}

void ModuleLoader::Shutdown()
{
    if (s_ModuleLoader != nullptr)
        delete s_ModuleLoader;
    s_ModuleLoader = nullptr;
}

bool ModuleLoader::CheckModuleHasInMemory(const char* name)
{
    FILE* fp = fopen("/proc/self/maps", "r");
    if (fp == NULL)
    {
        perror(__FUNCTION__);
        return false;
    }

    bool found = false;
    
    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (strstr(line, name) == NULL)
            continue;

        found = true;
        break;
    }
    fclose(fp);
    return found;
}

ModuleLoader::ModuleLoader()
{
    logger::info("[ ModuleLoader ] Initialize '%p'.", this);
}

ModuleLoader::~ModuleLoader()
{
    logger::info("[ ModuleLoader ] Shutdown '%p'.", this);
}

void ModuleLoader::load(const std::string& name, const std::string& path)
{
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (handle == nullptr)
        throw std::runtime_error(dlerror());

    m_moduleMap.emplace(name, handle);
}

void ModuleLoader::unload(const std::string& name)
{
    auto it = m_moduleMap.find(name);
    if (it == m_moduleMap.end())
        return;

    if (dlclose(it->second) != 0)
        throw std::runtime_error(dlerror());

    m_moduleMap.erase(it);
}

void ModuleLoader::unload()
{
    for (auto&& [name, handle] : m_moduleMap)
    {
        if (dlclose(handle) != 0)
            throw std::runtime_error(dlerror());
    }
    m_moduleMap.clear();
}

void* ModuleLoader::getModule(const std::string& name)
{
    auto it = m_moduleMap.find(name);
    if (it == m_moduleMap.end())
        return nullptr;
    return it->second;
}

void* ModuleLoader::findSymbol(const std::string& module, const std::string& name)
{
    auto handle = getModule(module);
    if (handle == nullptr)
        return nullptr;

    void* addr = dlsym(handle, name.c_str());
    if (addr == nullptr)
        throw std::runtime_error(dlerror());

    return addr;
}


