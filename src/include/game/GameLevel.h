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
#include <unordered_map>
#include <queue>

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
    bool bCompleted;
	bool bLocked;
    std::string sTileMap;
	std::string sBackground;

	GameLevelInfo() 
		: sName(""), nDifficulty(0), bCompleted(false), bLocked(true), sTileMap(""), nLevel(0) {}
};

/*
* Represents game level.
* Note: Implements observer pattern. Can observe player and it can be observed by game (for example.)
*/
class GameLevel : public IObserver, public BasicObserverSubject
{
public:
	GameLevelInfo* Info = nullptr;
	Physics2D::PhysicsWorld* PhysicsWorld = nullptr;
	Tilemap* Map = nullptr;
	Texture2D* Background = nullptr;
	Player* pPlayer = nullptr;

	GameLevel() {}

	void ProcessInput(InputInterface* input, float dt);
	void Update(float dt);
	void Render(SpriteRenderer* pSpriteRenderer, TilemapRenderer* pTilemapRenderer);

	void Load(GameLevelInfo* pInfo);
	void Unload();

	// Observer implementation.
	void OnNotify(IObserverSubject* obj, int message, void* args = nullptr);

protected:
	// Binding of coins to physics2D objects (eg. RigidBody -> tile_gid).
	std::unordered_map<Physics2D::RigidBody*, MapTileInfo> coins;
	// Event queue;
	struct Event { IObserverSubject* sender; int message; void* args; };
	std::queue<Event> eventQueue;
	std::array<int, 4> acceptedMessages = {
		PLAYER_HIT_SPIKES,
		PLAYER_JUMPED,
		PLAYER_LANDED,
		PLAYER_COLLIDE_COIN
	};
	// Initial position of the player in tile-space.
	glm::vec2 vInitPlayerPosition = glm::vec2(0.0f, 0.0f);

	void init_physics_world();
	void init_tilemap();
	void init_world_objects();
	void init_background();
	void init_player();
	void init_tilecamera();
	void handle_events(float dt);
};

void to_json(nlohmann::json& j, const GameLevelInfo& info);
void from_json(const nlohmann::json& json, GameLevelInfo& info);
void to_json(nlohmann::json& json, const std::vector<GameLevelInfo>& infos);
void from_json(const nlohmann::json& json, std::vector<GameLevelInfo>& infos);