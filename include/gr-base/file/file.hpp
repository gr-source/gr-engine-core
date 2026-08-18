#pragma once

#include <cstdint>
#include <string>

namespace gfile
{
    std::string create_temp_file(const std::string& name);

    std::string get_home_directory();

    const std::string& get_data_directory();

    const std::string& get_config_directory();

    const std::string& get_cache_directory();

    const std::string& get_current_source_dir();

    bool exists(const std::string &path);

    bool is_directory(const std::string &path);

    bool create_directory(const std::string &path);

    bool create_directories(const std::string &path);

    bool remove(const std::string &path);

    std::string parent(const std::string &path);

    std::string get_file_name(const std::string &path);

    bool move(const char* src_path, const char* dest_path);

    bool copy(const std::string& src, const std::string& dst);

    uint8_t* read(const char* file, size_t *length);
};



