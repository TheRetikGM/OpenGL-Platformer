#pragma once
#include <Tmx.h>
#include <map>
#include <vector>
#include <list>
#include "texture.h"
#include "interfaces/IAnimated.h"
#include <algorithm>

class AnimatedTile;

struct MapTileInfo
{
	std::string layer_name = "";
	int x = 0, y = 0;

	MapTileInfo(std::string tilelayer, int x, int y) : layer_name(tilelayer), x(x), y(y) {}
	MapTileInfo() = default;
	
	friend bool operator==(const MapTileInfo& lhs, const MapTileInfo& rhs);
};

inline bool operator==(const MapTileInfo& lhs, const MapTileInfo& rhs)
{
	return rhs.layer_name == lhs.layer_name && rhs.x == lhs.x && rhs.y == lhs.y; 
}

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
	std::string GetFilePath() { return file_path; }

	void HideTile(MapTileInfo inf) { if (!IsHidden(inf)) hidden_tiles.emplace_back(inf); };
	void HideTile(std::string layer_name, int x, int y) { HideTile(MapTileInfo(layer_name, x, y)); };
	void ShowTile(MapTileInfo inf) { hidden_tiles.remove(inf); }
	void ShowTile(std::string layer_name, int x, int y) { hidden_tiles.remove(MapTileInfo(layer_name, x, y)); }
	bool IsHidden(MapTileInfo inf) const { return std::find(hidden_tiles.begin(), hidden_tiles.end(), inf) != hidden_tiles.end(); }
	bool IsHidden(std::string layer_name, int x, int y) const { return std::find(hidden_tiles.begin(), hidden_tiles.end(), MapTileInfo(layer_name, x, y)) != hidden_tiles.end(); }

private:
	std::list<MapTileInfo> hidden_tiles;
	std::string directory;
	std::string file_path;

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
	virtual ~AnimatedTile();
	AnimatedTile& operator=(const AnimatedTile& tile);
	
	void Update(float dt) override;
	int GetCurrentTileID();

private:
	float duration_left;
};
