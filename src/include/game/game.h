#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <algorithm>
#include <vector>
#include "Interfaces/ITileSpace.h"
#include <Tmx.h>

enum class GameState : uint8_t {
	active,
	menu,
	loading
};

enum class Direction : uint8_t {
	up = 0,
	down = 1,
	left = 2,
	right = 3
};

class Game
{
public:
	GameState		State;
	bool			Keys[1024];
	bool			KeysProcessed[1024];
	unsigned int	Width, Height;
	glm::vec3		BackgroundColor;

	static glm::vec2 TileSize;

	Game(unsigned int width, unsigned int height);
	~Game();

	void Init();	

	void ProcessInput(float dt);
	void ProcessMouse(float xoffset, float yoffset);
	void ProcessScroll(float yoffset);
	void Update(float dt);
	void Render();
	void OnLayerRendered(const Tmx::Map* map, const Tmx::Layer* layer, int n_layer);

	static void SetTileSize(glm::vec2 new_size);
	static void AddTileSpaceObject(ITileSpace* obj) { tileSpaceObjects.push_back(obj); };
	static void RemoveTileSpaceObject(ITileSpace* obj) { tileSpaceObjects.erase(std::remove(tileSpaceObjects.begin(), tileSpaceObjects.end(), obj), tileSpaceObjects.end()); };

	void OnResize();
protected:
	static std::vector<ITileSpace*> tileSpaceObjects;
};