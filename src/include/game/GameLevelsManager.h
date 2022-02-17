#pragma once
#include "game/GameLevel.h"
#include <vector>
#include <string>
#include "BasicObserverSubject.hpp"

/*
* Stores game progress.
*/
struct LevelProgress
{
	bool bCompleted = false;
	int nCoins = 0;
	int nMaxCoins = 0;
	bool bLocked = true;
	LevelProgress() = default;
};

class GameLevelsManager : public BasicObserverSubject, public IObserver
{
public:
	GameLevelsManager(const char* levels_json);
	virtual ~GameLevelsManager();

	// Loads level.
	void Load(int nLevel);
	// Info about active level.
	const GameLevelInfo& GetInfo() const { return level_infos[nActiveLevel]; }
	const std::vector<GameLevelInfo>& GetAllInfos() const { return level_infos; }
	// Check if any level is loaded.
	inline bool Active() const { return pActiveLevel != nullptr; }
	// Save current level progresses.
	void Save();
	// Free level resources and unload the level.
	void Unload();
	// Reset all level progress and revert to default state.
	void DeleteSavedStatistics();
	// Change the completed attribute of active level.
	void Completed(int nLevel, bool b) { 
		mLevelProgresses[nLevel].bCompleted = b; 
		notify(LEVEL_COMPLETED_CHANGED, nLevel);
	}
	// Set lock attribute of given level.
	void Locked(int nLevel, bool b = true) { 
		mLevelProgresses[nLevel].bLocked = b;
		notify(LEVEL_LOCKED_CHANGED, nLevel);
	}
	GameLevel& ActiveLevel() { return *pActiveLevel; }
	int ActiveLevelIndex() { return nActiveLevel; }
	GameLevelInfo& GetLevelInfo(int nLevel);

	LevelProgress& GetLevelProgress(int nLevel) { return mLevelProgresses[nLevel]; }

	// Implementation of IObserver
	void OnNotify(IObserverSubject* obj, int message, std::any args = nullptr) { notify(message, args); }

protected:
	std::vector<GameLevelInfo> level_infos;
	std::unordered_map<int, LevelProgress> mLevelProgresses;
	int nActiveLevel = 0;
	GameLevel* pActiveLevel = nullptr;
	std::string sJsonFile = "";	
	std::string sSaveName = "default.json";

	// Load level progresses from filesystem.
	// If they do not exists, then create a default ones.
	void load_level_progresses();
	// Creates default level progresses.
	void create_initial_progress();
};