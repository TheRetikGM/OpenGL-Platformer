#include "game/GameLevel.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <sstream>
#include "config.h"
#include <exception>
#include <stdexcept>
#include <iostream>

using nlohmann::json;

void from_json(const json& j, GameLevel& m)
{
	 j.at("name").get_to(m.Name);
	std::string tilemap_path = j.at("tilemap").get<std::string>();
	try {
		m.Map = new Tilemap((std::string(ASSETS_DIR) + tilemap_path).c_str());
	}
	catch (const std::runtime_error& err) {
		throw std::runtime_error(std::string("GameLevel::Load()::from_json(): ") + err.what());
	}
}
void to_json(json& j, const GameLevel& m)
{
	j["name"] = m.Name;
	j["tilemap"] = m.Map->GetFilePath();
}
GameLevel* GameLevel::Load(const char* path)
{
	// Load raw data from file
	std::ifstream jsonFile(path);
	if (!jsonFile.is_open())
		throw std::runtime_error("Could not load GameLevel at '" + std::string(path) + "'.");
	std::stringstream json_data;
	json_data << jsonFile.rdbuf();

	// Parse json structure
	nlohmann::json j;
	json_data >> j;

	// Create level from json structure.
	GameLevel* level = new GameLevel();	
	auto level_local = j.get<GameLevel>();
	level->Name = level_local.Name;
	level->PhysicsWorld = level_local.PhysicsWorld;
	level->Map = level_local.Map;
	glm::vec2 map_size = glm::vec2((float)level->Map->Map->GetWidth(), (float)level->Map->Map->GetHeight());
	level->PhysicsWorld = new Physics2D::PhysicsWorld(glm::vec2(0.0f), map_size, 1.0f);
	// level->PhysicsWorld->CollisionTree->SortOnTests = true;
	level->LoadObjectsFromTilemap();

	return level;
}
void GameLevel::Delete(GameLevel* level)
{
	delete level->Map;
	delete level->PhysicsWorld;
	delete level;
}

void GameLevel::LoadObjectsFromTilemap()
{
	Tmx::Map* map = Map->Map;

	for (int layer_i = 0; layer_i < map->GetNumTileLayers(); layer_i++)
	{
		auto layer = map->GetTileLayer(layer_i);
		for (int i = 0; i < layer->GetWidth() * layer->GetHeight(); i++)
		{
			int x = i % layer->GetWidth();
			int y = i / layer->GetWidth();
				
			int tileset_index = layer->GetTileTilesetIndex(x, y);
			if (tileset_index == -1)
				continue;

			auto set = map->GetTileset(tileset_index);
			auto tile = set->GetTile(layer->GetTileId(x, y));
			if (tile && tile->HasObjects())
			{
				for (Tmx::Object* obj : tile->GetObjects())
				{
					const Tmx::Polygon* polygon = obj->GetPolygon();
					glm::vec2 set_tile_size = glm::vec2(set->GetTileWidth(), set->GetTileHeight());
					glm::vec2 tilespace_pos = glm::vec2(obj->GetX(), obj->GetY()) / set_tile_size + glm::vec2(x, y);
					glm::vec2 local_size = glm::vec2(obj->GetWidth(), obj->GetHeight()) / set_tile_size;
					Physics2D::RigidBody* body = nullptr;

					if (polygon)
					{
						std::vector<glm::vec2> points;
						for (int p = 0; p < polygon->GetNumPoints(); p++)
						{
							const Tmx::Point point = polygon->GetPoint(p);
							points.push_back(tilespace_pos + glm::vec2(point.x, point.y) / set_tile_size);
						}
						body = this->PhysicsWorld->AddPolygonBody(tilespace_pos, points, 2.0f, true, 1.0f);
					}
					else {
						body = this->PhysicsWorld->AddRectangleBody(tilespace_pos, local_size, 2.0f, true, 1.0f);
					}

					if (body)
						body->Name = obj->GetProperties().GetStringProperty("name");
				}
			}
		}
	}
}