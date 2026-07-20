#include "gr-base/core/file/file.hpp"

#include <filesystem>
#include <cstdlib>
#include <cstdio>

#include <sys/stat.h>

#include <string.h>
#include <dirent.h>
#include <unistd.h>

// ********** gfile ********** //
std::string gfile::create_temp_file(const std::string &name)
{
    return std::filesystem::temp_directory_path().string() + "/" + name;
}

std::string gfile::get_home_directory()
{
    static const char *retval = getenv("HOME");
    return retval != NULL ? retval : "/";
}

const std::string& gfile::get_data_directory()
{
    static std::string dir = get_home_directory() + "/.local/share/gr-engine";
    return dir;
}

const std::string& gfile::get_config_directory()
{
    static std::string dir = get_home_directory() + "/.config/gr-engine";
    return dir;
}

const std::string& gfile::get_cache_directory()
{
    static std::string dir = get_home_directory() + "/.cache/gr-engine";
    return dir;
}

static std::string GetCWD()
{
    char path[512]{0};
    return getcwd(path, sizeof(path)) == path ? path : gfile::get_home_directory();
}

const std::string& gfile::get_current_source_dir()
{
    static std::string dir = GetCWD();
    return dir;
}

bool gfile::exists(const std::string &path)
{
    struct stat sb;
    return stat(path.c_str(), &sb) != -1 ? true : false;
}

bool gfile::is_directory(const std::string &path)
{
    struct stat sb;
    return stat(path.c_str(), &sb) != -1 && S_ISDIR(sb.st_mode);
}

bool gfile::create_directory(const std::string &path)
{
    return mkdir(path.c_str(), 0755) != -1 ? true : false;
}

bool gfile::create_directories(const std::string &path)
{
    std::string buffer(path.length(), 0);

    const char* ptr = path.c_str();
    size_t len = 0;

    while (*ptr)
    {
        buffer[len] = *ptr;

        if (*ptr == '/' && len > 0)
        {
            buffer[len] = '\0';
            if (!exists(buffer))
                if (!create_directory(buffer.c_str()))
                    return false;

            buffer[len] = '/';
        }
        len++;
        ptr++;
    }

    if (len > 0 && !create_directory(buffer.c_str()))
        return false;

    return true;
}

bool gfile::remove(const std::string &path)
{
    if (!gfile::exists(path))
        return false;

    bool result = true;

    if (gfile::is_directory(path))
    {
        DIR* dir = opendir(path.c_str());
        if (dir == NULL)
            return false;

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char string[1024];
            sprintf(string, "%s/%s", path.c_str(), entry->d_name);

            if (!gfile::remove(string))
            {
                result = false;
                break;
            }
        }

        closedir(dir);

        if (result)
            result = (rmdir(path.c_str()) == 0);
    } else
    {
        result = (unlink(path.c_str()) == 0);
    }

    return result;
}

std::string gfile::parent(const std::string &path)
{
    size_t start = path.rfind('/');
    if (start == std::string::npos)
        return "";

    if (start == 0)
        return "/";
    return path.substr(0, start);
}

std::string gfile::get_file_name(const std::string &path)
{
    const char* token = strrchr(path.c_str(), '/');
    if (token == NULL)
        return path;

    return token + 1;
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
        std::filesystem::copy_file(src_path, dest_path);
    }
    return true;
}

uint8_t* gfile::read(const char* file, size_t *length)
{
    uint8_t* buffer = NULL;

    FILE* fp = fopen(file, "rb");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        *length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        buffer = (uint8_t*)malloc(*length + 1);
        if (fread(buffer, 1, *length, fp) == *length)
            buffer[*length] = 0;

        fclose(fp);
    }

    return buffer;
}



