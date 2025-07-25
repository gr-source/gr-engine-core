#pragma once

#include <vector>
#include <string>
#include <ctime>

class File {
public:
    static void SetCurrentProject(const std::string &path);

    static const std::string &GetCurrentProject();

    static void SetConfigPath(const std::string &path);

    static void SetCachePath(const std::string &path);

    static void SetUserPath(const std::string &path);

    static std::string GetUserFile(const std::string &path);

    static std::string GetCacheFile(const std::string &path);

    static std::string GetFileContent(const std::string &path);

    static bool Exists(const std::string& path);

    static std::string GetFileName(const std::string &path);

    static std::string GetFile(const std::string &path, bool internal = false);

    static std::string GetAsset(const std::string &path, bool internal = false);

    static bool CreateDirectories(const std::string &path);

    static bool CreateDirectory(const std::string &path);

    static bool Delete(const std::string &path);

    static bool Move(const std::string &from, const std::string &to);

    static bool Copy(const std::string &from, const std::string &to);

    static std::string getHomeDirectory();

    static std::string GetError();

    static std::string GetTempFile(const std::string &path);

    static void rename(const std::string &path, const std::string &name);
    
private:
    static std::string m_currentPath;

    static std::string m_configPath;

    static std::string m_cachePath;

    static std::string m_userPath;

    static std::stringstream m_error;
};


class gfile {
public:
    static std::string AssetStreamPath;

    static std::string create_temp_file(const std::string& name);

    static std::string get_home_directory();

    static std::string get_data_directory(const std::string &path = "");

    static std::string get_config_directory(const std::string &path = "");

    static std::string get_cache_directory();

    static bool exists(const std::string &path);

    static bool is_directory(const std::string &path);

    static bool create_directories(const std::string &path);

    static bool create_directory(const std::string &path);

    static bool remove(const std::string &path);

    static std::string get_parent(const std::string &path);
};



