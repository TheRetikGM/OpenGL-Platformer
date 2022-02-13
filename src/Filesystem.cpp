#include "Filesystem.h"
#include <filesystem>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <sstream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static const std::string LOCAL_FILES_DIR=std::string(getenv("appdata")) + "\\OpenGL-Platformer";
#else
static const std::string LOCAL_FILES_DIR=std::string(getenv("HOME")) + "/.local/share/OpenGL-Platformer";
#endif

namespace fs = std::filesystem;

Filesystem& Filesystem::Instance() 
{ 
    if (!instance)
        instance = std::shared_ptr<Filesystem>(new Filesystem);
    return *instance.get();
}
Filesystem::Filesystem()
{
    paLocalfiles = fs::path{LOCAL_FILES_DIR};

    if (fs::exists(paLocalfiles))
        return;
    
    // Create game directory structure.
    fs::create_directories(paLocalfiles/"saves");
}
std::string Filesystem::GetLocalFilesPath()
{
    return LOCAL_FILES_DIR;
}

void Filesystem::Save(const std::string& save_data, std::string save_filename)
{
    fs::path path = paLocalfiles/"saves"/save_filename;
    std::ofstream ofs{path};

    if (!ofs.is_open())
        throw std::runtime_error("Filesystem::Save(): Cannot open file '" + path.string() + "' for writing.");

    ofs << save_data;
    ofs.close();
}

std::string Filesystem::LoadRawSave(std::string save_filename)
{
    fs::path path = paLocalfiles/"saves"/save_filename;
    std::ifstream ifs{path};
    
    if (!ifs.is_open())
        throw std::runtime_error("Filesystem::LoadSave(): Cannot open file '" + path.string() + "' for reading.");
    
    std::stringstream buf;
    buf << ifs.rdbuf();
    std::string save_data = buf.str();
    ifs.close();

    return save_data;
}

bool Filesystem::SaveExists(std::string save_filename)
{
    return fs::exists(paLocalfiles/"saves"/save_filename);    
}
