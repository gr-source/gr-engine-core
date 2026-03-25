#pragma once

#include <vector>
#include <string>
#include <ctime>


class gfile {
public:
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

    static const char* getname(const char* path);

    static bool move(const char* src_path, const char* dest_path);

    static bool copy(const char* src_path, const char* dest_path);
};



