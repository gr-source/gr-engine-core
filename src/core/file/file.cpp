#include "gr-base/core/file/file.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>

// ********** gfile ********** //
std::string gfile::create_temp_file(const std::string &name)
{
    return std::filesystem::temp_directory_path().string() + "/" + name;
}

std::string gfile::get_home_directory()
{
    const char *retval = getenv("HOME");
    return retval != nullptr ? retval : "";
}

std::string gfile::get_data_directory(const std::string &path)
{
    return get_home_directory() + "/.local/share/gr-engine" + path;
}

std::string gfile::get_config_directory(const std::string &path)
{
    return get_home_directory() + "/.config/gr-engine" + path;
}

std::string gfile::get_cache_directory()
{
    return get_home_directory() + "/.cache/gr-engine";
}

bool gfile::exists(const std::string &path)
{
    return std::filesystem::exists(path);
}

bool gfile::is_directory(const std::string &path)
{
    return std::filesystem::is_directory(path);
}

bool gfile::create_directories(const std::string &path)
{
    try
    {
        std::filesystem::create_directories(path);
    } catch (const std::exception &e)
    {
        return false;
    }
    return true;
}

bool gfile::create_directory(const std::string &path)
{
    try
    {
        std::filesystem::create_directory(path);
    } catch (const std::exception &e)
    {
        return false;
    }
    return true;
}

bool gfile::remove(const std::string &path)
{
    if (!std::filesystem::is_directory(path))
    {
        return std::filesystem::remove(path);
    }

    for (auto entry : std::filesystem::directory_iterator(path))
    {
        if (!remove(entry.path()))
        {
            return false;
        }
    }
    return std::filesystem::remove(path);
}

std::string gfile::get_parent(const std::string &path)
{
    auto start = path.rfind('/');
    if (start == std::string::npos)
        return path;
    return path.substr(0, start);
}

const char* gfile::getname(const char* path)
{
    const char* token = strrchr(path, '/');
    return token != NULL ? token + 1 : path;
}

bool gfile::move(const char* src_path, const char* dest_path)
{
    if (!copy(src_path, dest_path))
        return false;

    return remove(src_path);
}

bool gfile::copy(const char* src_path, const char* dest_path)
{
    if (gfile::is_directory(src_path))
    {
        if (!gfile::exists(dest_path))
            gfile::create_directory(dest_path);

        for (auto& entry : std::filesystem::directory_iterator(src_path))
        {
            char path[512];
            sprintf(path, "%s/%s", dest_path, entry.path().filename().c_str());

            if (entry.is_directory())
            {
                if (!copy(entry.path().c_str(), path))
                    break;
            } else
            {
                std::filesystem::copy_file(entry, path);
            }
        }
    } else
    {
        return false;
    }
    return true;
}




