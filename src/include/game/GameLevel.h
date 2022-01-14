#pragma once
#include "PhysicsWorld.h"
#include "tilemap.h"
#include "game/Player.h"
#include "nlohmann/json.hpp"

class level_locked_exception : public std::runtime_error
{
public:
	level_locked_exception(const char* msg = "") : std::runtime_error(msg) {}
	level_locked_exception(std::string msg = "") : std::runtime_error(msg) {}
};

struct GameLevelInfo
{
    std::string sName;
    int nDifficulty;
    bool bCompleted;
	bool bLocked;
    std::string sTileMap;

	GameLevelInfo() 
		: sName(""), nDifficulty(0), bCompleted(false), bLocked(true), sTileMap("") {}
};

class GameLevel
{
public:
	GameLevelInfo* Info = nullptr;
	Physics2D::PhysicsWorld* PhysicsWorld = nullptr;
	Tilemap* Map = nullptr;

	GameLevel() {}

	void Update(float dt);
	void Load(GameLevelInfo* pInfo);
	void Unload();

protected:
	void init_physics_world();
	void init_tilemap();
	void init_world_objects();
};

void to_json(nlohmann::json& j, const GameLevelInfo& info);
void from_json(const nlohmann::json& json, GameLevelInfo& info);
void to_json(nlohmann::json& json, const std::vector<GameLevelInfo>& infos);
void from_json(const nlohmann::json& json, std::vector<GameLevelInfo>& infos);