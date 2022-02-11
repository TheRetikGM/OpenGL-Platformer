#pragma once
#include "PhysicsWorld.h"
#include "tilemap.h"
#include "game/Player.h"
#include "nlohmann/json.hpp"
#include "InputInterface.hpp"
#include "sprite_renderer.h"
#include "tilemap_renderer.h"
#include "BasicObserverSubject.hpp"
#include "game/GameEvents.h"
#include "game/SingleAnimations.h"
#include "game/InGameHUD.h"
#include "game/Dialog.hpp"
#include <unordered_map>
#include <queue>
#include <memory>

class level_locked_exception : public std::runtime_error
{
public:
	level_locked_exception(const char* msg = "") : std::runtime_error(msg) {}
	level_locked_exception(std::string msg = "") : std::runtime_error(msg) {}
};

struct GameLevelInfo
{
	int nLevel;
    std::string sName;
    int nDifficulty;
	int nLives;
    bool bCompleted;
	bool bLocked;
    std::string sTileMap;
	std::string sBackground;
	std::string sSingleAnimationsPath;

	GameLevelInfo() 
		: sName(""), nDifficulty(0), bCompleted(false), bLocked(true), sTileMap(""), nLevel(0), sSingleAnimationsPath("") {}
};

/*
* Represents game level.
* Note: Implements observer pattern. Can observe player and it can be observed by game (for example.)
*/
class GameLevel : public IObserver, public BasicObserverSubject
{
public:
	GameLevelInfo* 				Info = nullptr;
	Physics2D::PhysicsWorld* 	PhysicsWorld = nullptr;
	Tilemap* 					Map = nullptr;
	Texture2D* 					Background = nullptr;
	Player* 					pPlayer = nullptr;
	SingleAnimations* 			pSingleAnimations = nullptr;
	InGameHUD* 					pHUD = nullptr;

	GameLevel() : nCoins(0), nCoinsTotal(0) {}

	void ProcessInput(InputInterface* input, float dt);
	void Update(float dt);
	void Render(SpriteRenderer* pSpriteRenderer, TilemapRenderer* pTilemapRenderer);

	void Load(GameLevelInfo* pInfo);
	void Unload();
	void Restart() {
		assert(Info != nullptr);
		nCoins = nCoinsTotal = 0;
		fElapsedTime = 0.0f;
		bPlayerDied = bPlayerDying = false;
		GameLevelInfo* tmp = Info;
		Unload();
		Load(tmp);
		notify(LEVEL_RESTARTED);
	}
	void OnPlayerDied();

	void OnResize();

	// Observer implementation.
	void OnNotify(IObserverSubject* obj, int message, void* args = nullptr);

	int GetCoins() const { return nCoins; }
	int GetCoinsTotal() const { return nCoinsTotal; }
	float GetElapsedTime() const { return fElapsedTime; }

protected:
	// Elapsed time in seconds.
	float fElapsedTime = 0.0f;
	// Binding of coins to physics2D objects (eg. RigidBody -> tile_gid).
	std::unordered_map<Physics2D::RigidBody*, MapTileInfo> coins;
	// Amount of collected coins. Should be equal to above coins.size().
	int nCoins = 0;
	// Total amount of coins.
	int nCoinsTotal = 0;
	// Event queue;
	std::queue<Event> eventQueue;
	std::array<int, 6> acceptedMessages = {
		PLAYER_LOST_LIFE,
		PLAYER_JUMPED,
		PLAYER_LANDED,
		PLAYER_COLLIDE_COIN,
		PLAYER_WALL_JUMPED,
		PLAYER_REACHED_FINISH
	};
	// Initial position of the player in tile-space.
	glm::vec2 vInitPlayerPosition = glm::vec2(0.0f, 0.0f);
	// Atlas text renderer for rendering in-game text.
	AtlasTextRenderer* pTextRenderer = nullptr;
	// Handle player dying stage.
	bool bPlayerDied = false;
	bool bPlayerDying = false;

	void init_physics_world();
	void init_tilemap();
	void init_world_objects();
	void init_background();
	void init_player();
	void init_tilecamera();
	void init_single_animations();
	void init_hud();

	void handle_events(float dt);
	void pickup_coin(Physics2D::RigidBody* coin);

	friend class InGameHUD;
};

// Just to make sure that serializatin functions are defined in
// the same namespace as GameLevelInfo.
void to_json(nlohmann::json& j, const GameLevelInfo& info);
void from_json(const nlohmann::json& json, GameLevelInfo& info);
void to_json(nlohmann::json& json, const std::vector<GameLevelInfo>& infos);
void from_json(const nlohmann::json& json, std::vector<GameLevelInfo>& infos);