#pragma once

#include <string>

namespace gfile
{
    std::string create_temp_file(const std::string& name);

    std::string get_home_directory();

    std::string get_data_directory(const std::string &path = "");

    std::string get_config_directory(const std::string &path = "");

    std::string get_cache_directory();

    bool exists(const std::string &path);

    bool is_directory(const std::string &path);

    bool create_directories(const std::string &path);

    bool create_directory(const std::string &path);

    bool remove(const std::string &path);

    std::string get_parent(const std::string &path);

    const char* getname(const char* path);

    bool move(const char* src_path, const char* dest_path);

    bool copy(const char* src_path, const char* dest_path);
};



