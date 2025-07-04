#include "gr-base/core/file/file.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>

std::string File::m_currentPath;

std::string File::m_configPath;

std::string File::m_cachePath;

std::string File::m_userPath;

std::stringstream File::m_error;

void File::SetCurrentProject(const std::string &path)
{
    m_currentPath = path;
}

const std::string &File::GetCurrentProject() {
    return m_currentPath;
}

void File::SetConfigPath(const std::string &path) {
    m_configPath = path;
}

void File::SetCachePath(const std::string &path) {
    m_cachePath = path;
}

void File::SetUserPath(const std::string &path) {
    m_userPath = path;
}

std::string File::GetUserFile(const std::string &path) {
    return m_userPath + path;
}

std::string File::GetCacheFile(const std::string &path) {
    return m_cachePath + path;
}

std::string File::GetFileContent(const std::string &path) {
    std::ifstream is(path);
    if (!is.is_open())
        return "";
        
    std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    is.close();
    return content;
}

bool File::Exists(const std::string &path) {
    return std::filesystem::exists(path);
}

std::string File::GetFileName(const std::string &path) {
    auto fileName = std::filesystem::path(path).filename();
    return fileName.replace_extension().string();
}

std::string File::GetFile(const std::string &path, bool internal) {
    return (internal ? m_userPath : m_currentPath) + path;
}

std::string File::GetAsset(const std::string &path, bool internal) {
    return GetFile("/assets" + path, internal);
}

bool File::CreateDirectories(const std::string &path) {
    return std::filesystem::create_directories(path);
}

bool File::CreateDirectory(const std::string &path) {
    return std::filesystem::create_directory(path);
}

bool File::Delete(const std::string &path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    std::filesystem::remove_all(path);
    return true;
}

bool File::Move(const std::string &from, const std::string &to) {
    if (!std::filesystem::is_directory(to)) {
        return false;
    }
    if (!Copy(from, to)) {
        return false;
    }
    
    if (!Delete(from)) {
        return false;
    }

    return true;
}

bool File::Copy(const std::string &from, const std::string &to) {
    if (!std::filesystem::exists(from)) {
        m_error << "Target directory or file do not exists: " << from << std::endl;
        return false;
    }

    if (!std::filesystem::is_directory(from)) {
        auto next = to + "/" + std::filesystem::path(from).filename().string();
        
        if (std::filesystem::is_directory(next)) {
            m_error << "Path: " << next << " is a directory" << std::endl;
            return false;
        }

        if (std::filesystem::exists(next)) {
            std::filesystem::remove(next);
        }
        
        std::filesystem::copy(from, to);
        return true;
    }
    
    auto newRoot = to + "/" + std::filesystem::path(from).filename().string();

    CreateDirectory(newRoot);

    auto isSucess = true;
    for (auto entry : std::filesystem::directory_iterator(from)) {
        auto name = entry.path().filename().string();

        isSucess = Copy(entry.path(), newRoot);
        if (!isSucess) {
            break;
        }
    }
    return isSucess;
}

std::string File::getHomeDirectory() {
    auto path = getenv("HOME");
    return std::string(path != nullptr ? path : "/");
}

std::string File::GetError() {
    auto temp = m_error.str();
    m_error.clear();
    return temp;
}

std::string File::GetTempFile(const std::string &path) {
    return std::filesystem::temp_directory_path().string() + "/" + path;
}

void File::rename(const std::string &from, const std::string &to) {
    std::filesystem::rename(from, to);
}

// ********** gfile ********** //

std::string gfile::AssetStreamPath;

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

std::string gfile::get_cache_directory(const std::string &path)
{
    return get_home_directory() + "/.cache/gr-engine" + path;
}

bool gfile::exists(const std::string &path)
{
    return std::filesystem::exists(path);
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




