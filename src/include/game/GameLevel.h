#pragma once
#include "PhysicsWorld.h"
#include "tilemap.h"
#include "game/Player.h"

struct GameLevelInfo
{
    std::string sName;
    int nDifficulty;
    bool bCompleted;
	bool bLocked;
    std::string sTileMap;
};

class GameLevelsManager;
class _GameLevel
{
public:
	GameLevelInfo* info = nullptr;
	Physics2D::PhysicsWorld* PhysicsWorld = nullptr;
	Tilemap* Map = nullptr;

	_GameLevel() {}

	void Update(float dt);
protected:
	void init_world_objects();

	friend class GameLevelsManager;
};

class GameLevelsManager
{
public:
	GameLevelsManager(const char* levels_json);

	// Loads level.
	void Load(int nLevel);
	// Info about active level.
	const GameLevelInfo& GetInfo() const { return level_infos[nActiveLevel]; }
	// Check if any level is loaded.
	inline bool Active() const { return pActiveLevel != nullptr; }
	// Save current level statistics.
	void Save();
	// Free level resources and unload the level.
	void Unload();
	// Reset all level progress and revert to default state.
	void DeleteSavedStatistics();

protected:
	std::vector<GameLevelInfo> level_infos;
	int nActiveLevel = 0;
	_GameLevel* pActiveLevel = nullptr;
};

class GameLevel
{
public:
	std::string Name;
	Physics2D::PhysicsWorld* PhysicsWorld;
	Tilemap* Map;

	GameLevel(std::string name, Physics2D::PhysicsWorld* world, Tilemap* map): Name(name), PhysicsWorld(world), Map(map) {}
	GameLevel() = default;
	~GameLevel() {} 

	static GameLevel* Load(const char* path);
	static void Delete(GameLevel* level);

	void Update(float dt)
	{
		PhysicsWorld->Update(dt, 1);
		Map->Update(dt);
	}

	void LoadObjectsFromTilemap();
protected:	
};