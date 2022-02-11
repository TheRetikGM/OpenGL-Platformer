#pragma once
#include "game/GameLevel.h"
#include <vector>
#include <string>
#include "BasicObserverSubject.hpp"

class GameLevelsManager : public BasicObserverSubject, public IObserver
{
public:
	GameLevelsManager(const char* levels_json);
	~GameLevelsManager();

	// Loads level.
	void Load(int nLevel);
	// Info about active level.
	const GameLevelInfo& GetInfo() const { return level_infos[nActiveLevel]; }
	const std::vector<GameLevelInfo>& GetAllInfos() const { return level_infos; }
	// Check if any level is loaded.
	inline bool Active() const { return pActiveLevel != nullptr; }
	// Save current level statistics.
	void Save();
	// Free level resources and unload the level.
	void Unload();
	// Reset all level progress and revert to default state.
	void DeleteSavedStatistics();
	// Change the completed attribute of active level.
	void Completed(int nLevel, bool b) { level_infos[nLevel].bCompleted = b; }
	// Set lock attribute of given level.
	void Locked(int nLevel, bool b = true) { level_infos[nLevel].bLocked = b; }
	GameLevel& ActiveLevel() { return *pActiveLevel; }
	int ActiveLevelIndex() { return nActiveLevel; }
	GameLevelInfo& GetLevelInfo(int nLevel);

	// Implementation of IObserver
	void OnNotify(IObserverSubject* obj, int message, void* args = nullptr) { notify(message, args); }

protected:
	std::vector<GameLevelInfo> level_infos;
	int nActiveLevel = 0;
	GameLevel* pActiveLevel = nullptr;
	std::string sJsonFile = "";
};