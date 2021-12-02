#pragma once
#include "PhysicsWorld.h"
#include "tilemap.h"
#include "game/Player.h"

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