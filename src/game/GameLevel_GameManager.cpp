#include "game/GameLevel.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <sstream>
#include "config.h"
#include <exception>
#include <stdexcept>
#include <iostream>

using nlohmann::json;

// ***********************************************
// TODO: Define serialization and deserialization.
// ***********************************************
void to_json(json& json, const GameLevelInfo& info)
{

}
void from_json(const json& json, GameLevelInfo& info)
{

}
void to_json(json& json, const std::vector<GameLevelInfo>& infos)
{

}
void from_json(const json& json, std::vector<GameLevelInfo>& infos)
{
    
}


void _GameLevel::Update(float dt)
{
    this->PhysicsWorld->Update(dt, 5.0f);
    Map->Update(dt);
}
void _GameLevel::init_world_objects()
{
    // TODO: init physics world objects.
}

GameLevelsManager::GameLevelsManager(const char* levels_json)
{
    // TODO: Deserialize levels json and init level infos
}
void GameLevelsManager::Save()
{
    // TODO: Save current level into level infos and serialize it into json file.
}
void GameLevelsManager::Unload()
{
    // TODO: Free GameLevel physics world and map, set active to null.
}
void GameLevelsManager::DeleteSavedStatistics()
{
    // TODO: delete all progress in level infos and serialize it
    // there should not be any active level.
}