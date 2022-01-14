#include "game/GameLevel.h"
#include "game/GameLevelsManager.h"
#include <fstream>
#include <sstream>
#include "config.h"
#include <exception>
#include <stdexcept>
#include <iostream>
#include <iomanip>

using nlohmann::json;

// ***********************************************
// Define JSON serialization and deserialization.
// ***********************************************
void to_json(json& j, const GameLevelInfo& info)
{
    j = json{
        {"name", info.sName},
        {"difficulty", info.nDifficulty},
        {"completed", info.bCompleted},
        {"locked", info.bLocked},
        {"tilemap", info.sTileMap}
    };
}
void from_json(const json& json, GameLevelInfo& info)
{
    json.at("name").get_to(info.sName);
    json.at("difficulty").get_to(info.nDifficulty);
    json.at("completed").get_to(info.bCompleted);
    json.at("locked").get_to(info.bLocked);
    json.at("tilemap").get_to(info.sTileMap);
}
void to_json(json& json, const std::vector<GameLevelInfo>& infos)
{
    for (int i = 0; i < int(infos.size()); i++)
        to_json(json[i], infos[i]);
}
void from_json(const json& json, std::vector<GameLevelInfo>& infos)
{
    for (auto& i : json)
        infos.push_back(i.get<GameLevelInfo>());
}

// ************************
// _GameLevel definitions
// ************************
void GameLevel::Update(float dt)
{
    this->PhysicsWorld->Update(dt, 5.0f);
    Map->Update(dt);
}
void GameLevel::Load(GameLevelInfo* pInfo)
{
    // Load level based on the info provided.
    this->Info = pInfo;
    init_tilemap();
    init_physics_world();
    init_world_objects();
}
void GameLevel::init_physics_world()
{
    this->PhysicsWorld = new Physics2D::PhysicsWorld(
        glm::vec2(0.0f, 0.0f),
        glm::vec2(float(Map->Map->GetWidth()), float(Map->Map->GetHeight())),
        1.0f
    );
}
void GameLevel::init_tilemap()
{
    this->Map = new Tilemap((ASSETS_DIR + Info->sTileMap).c_str());
}
void GameLevel::init_world_objects()
{
    // ==== Load objects from tilemap ====
    Tmx::Map* map = Map->Map;
    // For each layer
    for (int nLayer = 0; nLayer < map->GetNumTileLayers(); nLayer++)
    {
        const Tmx::TileLayer* layer = map->GetTileLayer(nLayer);
        for (int i = 0; i < layer->GetWidth() * layer->GetHeight(); i++)
        {
            int x = i % layer->GetWidth();
            int y = i / layer->GetWidth();

            int nTileset = layer->GetTileTilesetIndex(x, y);
            if (nTileset == -1)
                continue;

            const Tmx::Tileset* set = map->GetTileset(nTileset);
            const Tmx::Tile* tile = set->GetTile(layer->GetTileId(x, y));
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
                        // Translate polygon points to correct position.
                        std::vector<glm::vec2> points;
                        for (int nPoint = 0; nPoint < polygon->GetNumPoints(); nPoint++)
                        {
                            const Tmx::Point point = polygon->GetPoint(nPoint);
                            points.push_back(tilespace_pos + glm::vec2(point.x, point.y) / set_tile_size);
                        }
                        body = this->PhysicsWorld->AddPolygonBody(tilespace_pos, points, 2.0f, true, 1.0f);
					}
					else {
						body = this->PhysicsWorld->AddRectangleBody(tilespace_pos, local_size, 2.0f, true, 1.0f);
					}

                    // Assign body the additional properties set on the collider.
					if (body)
						body->Name = obj->GetProperties().GetStringProperty("name");
                }
            }
        }
    }
}
// Free all allocated resources for this level.
void GameLevel::Unload()
{
    // Check for null just in case some of the initializations failed.
    if (Map)
        delete Map;
    if (this->PhysicsWorld)
        delete PhysicsWorld;
    Info = nullptr;
}