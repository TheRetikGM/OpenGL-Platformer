#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <algorithm>
#include <vector>
#include "game/TileSpace.h"
#include <Tmx.h>
#include "PhysicsWorld.h"
#include "game/GameLevel.h"
#include "InputInterface.hpp"
#include "interfaces/Observer.h"
#include "game/Dialog.hpp"
#include "game/GameEvents.h"
#include <queue>

enum class GameState : uint8_t {
	active,
	loading,
	ingame_paused,
	ingame_dialog,
	main_menu
};

enum class Direction : uint8_t {
	up = 0,
	down = 1,
	left = 2,
	right = 3
};

class Game : public IObserver
{
public:
	GameState		State;
	bool			Keys[1024];
	bool			KeysProcessed[1024];
	unsigned int	Width, Height;
	glm::vec3		BackgroundColor;
	std::string		WindowTitle = "Game";
	bool 			Run = true;
	InputInterface* Input;

	static glm::vec2 TileSize;
	static glm::mat4 ProjectionMatrix;
	static glm::vec2 ScreenSize;

	Game(unsigned int width, unsigned int height);
	virtual ~Game();

	void Init();	

	void ProcessInput(float dt);
	void ProcessMouse(float xoffset, float yoffset);
	void ProcessScroll(float yoffset);
	void Update(float dt);
	void Render();

	static void SetTileSize(glm::vec2 new_size);

	void OnResize();

	// Implementation of Observer functions.
	virtual void OnNotify(IObserverSubject* obj, int message, std::any args = nullptr);

protected:
	std::unordered_map<std::string, Dialog> mDialogs;
	std::queue<Event> eventQueue;
	Dialog* pActiveDialog = nullptr;

	void handle_events();
	void init_dialogs();
	void recenter_forms();
	void exit_to_main_menu();
	void update_win_dialog_info();
};
