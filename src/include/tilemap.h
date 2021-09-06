#pragma once
#include <Tmx.h>
#include <map>
#include <vector>
#include "texture.h"
#include "IAnimated.h"

class AnimatedTile;

class Tilemap
{
public:
	Tmx::Map* Map;
	// tile gid - life
	std::map<int, AnimatedTile*> AnimTiles;	
	std::map<const Tmx::Tileset*, Texture2D> Tileset_textures;

	Tilemap(const char* path);
	~Tilemap();

	// Updates all animated tiles.
	void Update(float dt);
	int GetTileId(const Tmx::TileLayer* layer, int x, int y);

private:
	std::string directory;

	void loadTilemap(const char* path);
	void loadTextures();
	void initAnimatedTiles();
};

class AnimatedTile : public IAnimated
{
public:
	size_t CurrentFrame;
	const std::vector<Tmx::AnimationFrame> &Frames;

	AnimatedTile(const Tmx::Tile* tile);
	~AnimatedTile();
	AnimatedTile& operator=(const AnimatedTile& tile);
	
	void Update(float dt) override;
	int GetCurrentTileID();

private:
	float duration_left;
};