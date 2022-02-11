#include "game/GameLevelsManager.h"
#include <fstream>
#include <sstream>
#include "config.h"
#include <exception>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <iomanip>

using nlohmann::json;

GameLevelsManager::GameLevelsManager(const char* levels_json)
    : sJsonFile(levels_json)
{
    // ==== Deserialize levels json and init level infos ====
    std::ifstream ifs(levels_json);

    if (!ifs.is_open())
        throw std::runtime_error("GameLevelsManager(): Could not open file at: '" + std::string(levels_json) + "'.");

    json j;
    ifs >> j;
    this->level_infos = j.get<std::vector<GameLevelInfo>>();
    ifs.close();
}
GameLevelsManager::~GameLevelsManager()
{
    if (pActiveLevel)
        Unload();
}
void GameLevelsManager::Load(int nLevel)
{
    if (nLevel >= int(level_infos.size()))
        throw std::out_of_range("GameLevelsManager::Load(): Level index " + std::to_string(nLevel) + " out of range.");
    if (level_infos[nLevel].bLocked)
        throw level_locked_exception("Level with index " + std::to_string(nLevel) + " is locked.");

    // Load level using level info/
    if (pActiveLevel)
        Unload();
        
    pActiveLevel = new GameLevel();
    try
    {
        pActiveLevel->Load(&level_infos[nLevel]);
        pActiveLevel->AddObserver(this);
    }
    catch(const std::runtime_error& err)
    {
        // Free any resources allocated.
        Unload();
        throw std::runtime_error(std::string("GameLevelsManager::Load(): ") + err.what());
    }
    nActiveLevel = nLevel;
}
// Save current state of level infos.
void GameLevelsManager::Save()
{
    // Save current state of level infos on the disk.
    json j = level_infos;
    std::ofstream ofs(sJsonFile);
    ofs << std::setw(4) << j;
    ofs.close();
}
// Unload loaded level and free all its allocated resources.
void GameLevelsManager::Unload()
{
    // Free GameLevel physics world and map, set active to null.
    pActiveLevel->Unload();
    pActiveLevel->RemoveObserver(this);
    delete pActiveLevel;
    pActiveLevel = nullptr;
}
void GameLevelsManager::DeleteSavedStatistics()
{
    // Delete all progress in level infos and serialize it
    // there should not be any active level.
    for (auto& info : level_infos)
    {
        info.bCompleted = false;
        info.bLocked = false;
    }
    if (level_infos.size() >= 1)
        level_infos[0].bLocked = false;

    Save();
}
GameLevelInfo& GameLevelsManager::GetLevelInfo(int nLevel)
{
    if (nLevel >= level_infos.size())
        throw std::out_of_range("GameLevelsManager::GetLevelInfo(): Level index " + std::to_string(nLevel) + " out of range.");
    return level_infos[nLevel];
}