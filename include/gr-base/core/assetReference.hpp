#pragma once

#include "gr-base/externals/utils.h"

#include <string>
#include <unordered_map>

typedef uint64_t AssetHash;

class AssetReference
{
public:
    static void Initialize();

    static void Shutdown();

    static void Load(const std::string &file);

    static void Save(const std::string &file);

    static void Registry(const std::string &name, const std::string& file);

    static const std::string& GetFile(const std::string &name);

private:
    static AssetReference* s_AssetReference;

    std::unordered_map<std::string, std::string> m_refMapped;

    void load(const std::string &file);

    void save(const std::string &file);

    void registry(const std::string &name, const std::string& file);

    const std::string& getFile(const std::string& name);
};


