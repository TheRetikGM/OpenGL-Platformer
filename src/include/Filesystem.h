#pragma once
#include <memory>
#include <filesystem>

class Filesystem
{
public:
    static Filesystem& Instance();

    std::string GetLocalFilesPath();
    void Save(const std::string& save_data, std::string save_filename = "default.json");
    std::string LoadRawSave(std::string save_filename = "default.json");
    bool SaveExists(std::string save_filename);

private:
    inline static std::shared_ptr<Filesystem> instance = nullptr;

    std::filesystem::path paLocalfiles{};

    Filesystem();
};