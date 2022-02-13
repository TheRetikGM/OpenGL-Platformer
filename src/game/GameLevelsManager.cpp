#include "game/GameLevelsManager.h"
#include <fstream>
#include <sstream>
#include "config.h"
#include <exception>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <iomanip>
#include <iostream>
#include "Filesystem.h"

using nlohmann::json;

// ***********************************************
// Define JSON serialization and deserialization.
// ***********************************************
void from_json(const json& j, LevelProgress& p)
{
    j.at("completed").get_to(p.bCompleted);
    j.at("coins").get_to(p.nCoins);
    j.at("max_coins").get_to(p.nMaxCoins);
    j.at("locked").get_to(p.bLocked);
}
void to_json(json& j, const LevelProgress& p)
{
    j = json{
        {"completed", p.bCompleted},
        {"coins", p.nCoins},
        {"max_coins", p.nMaxCoins},
        {"locked", p.bLocked}
    };
}
void from_json(const json& j, std::unordered_map<int, LevelProgress>& p)
{
    for (const auto& prog : j.items())
        p[std::stoi(prog.key())] = prog.value().get<LevelProgress>();
}
void to_json(json& j, const std::unordered_map<int, LevelProgress>& p)
{
    for (auto& [sLevelID, progress] : p)
        j[std::to_string(sLevelID)] = progress;
}


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

    load_level_progresses();

    // The size of level_progresses must EXACTLY match
    // the size of level_infos.
    if (level_infos.size() != mLevelProgresses.size())
    {
        while(true)
        {
            std::cout << "Level infos changed! Press 'R' for default progress or 'C' to fix: ";
            std::string sInput = "";
            std::cin >> sInput;
            if (sInput == "R")
            {
                mLevelProgresses.clear();
                create_initial_progress();
                break;
            }
            else if (sInput == "C")
            {
                if (level_infos.size() > mLevelProgresses.size())
                {
                    for (int i = 0; i < int(level_infos.size()); i++)
                    {
                        if (mLevelProgresses.count(i) == 0)
                            mLevelProgresses[i] = LevelProgress();
                    }
                    break;
                }
                else
                    std::cout << "Fix failed." << std::endl;
            }
        }
        Save();
    }
}
GameLevelsManager::~GameLevelsManager()
{
    if (pActiveLevel)
        Unload();
}

void GameLevelsManager::load_level_progresses()
{
    Filesystem& fs = Filesystem::Instance();
    if (fs.SaveExists(sSaveName))
    {
        std::string json_data = fs.LoadRawSave(sSaveName);
        json j = json::parse(json_data);
        mLevelProgresses = j.get<std::unordered_map<int, LevelProgress>>();
    }
    else
    {
        create_initial_progress();
        Save();
    }
}
void GameLevelsManager::create_initial_progress()
{
    if (level_infos.empty())
            throw std::runtime_error("GameLevelsManager::load_level_progresses(): Level infos is empty.");
    // Create default save file.
    for (int i = 0; i < int(level_infos.size()); i++)
        mLevelProgresses[i] = LevelProgress();
    mLevelProgresses[0].bLocked = false;
}

void GameLevelsManager::Load(int nLevel)
{
    if (nLevel >= int(level_infos.size()))
        throw std::out_of_range("GameLevelsManager::Load(): Level index " + std::to_string(nLevel) + " out of range.");
    if (mLevelProgresses[nLevel].bLocked)
        throw level_locked_exception("Level with index " + std::to_string(nLevel) + " is locked.");

    // Load level using level info/
    if (pActiveLevel)
        Unload();
        
    pActiveLevel = new GameLevel();
    try
    {
        pActiveLevel->Load(&level_infos[nLevel]);
        pActiveLevel->AddObserver(this);
        mLevelProgresses[nLevel].nMaxCoins = pActiveLevel->GetCoinsTotal();
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
    if (pActiveLevel)
    {
        LevelProgress& prog = mLevelProgresses[nActiveLevel];
        if (pActiveLevel->bCompleted && !prog.bCompleted)
        {
            Completed(nActiveLevel, true);
            if (mLevelProgresses.count(nActiveLevel + 1) != 0)
                if (mLevelProgresses[nActiveLevel + 1].bLocked)
                    Locked(nActiveLevel + 1, false);
        }
        if (pActiveLevel->GetCoins() > prog.nCoins)
            prog.nCoins = pActiveLevel->GetCoins();
    }
    // Save current state of level infos on the disk.
    json j = mLevelProgresses;
    std::stringstream buf;
    buf << std::setw(4) << j;
    Filesystem::Instance().Save(buf.str(), sSaveName);
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
    create_initial_progress();
    Save();
    notify(LEVEL_PROGRESS_RESETED);
}
GameLevelInfo& GameLevelsManager::GetLevelInfo(int nLevel)
{
    if (nLevel >= level_infos.size())
        throw std::out_of_range("GameLevelsManager::GetLevelInfo(): Level index " + std::to_string(nLevel) + " out of range.");
    return level_infos[nLevel];
}