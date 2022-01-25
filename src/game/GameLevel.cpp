#include "game/GameLevel.h"
#include "game/GameLevelsManager.h"
#include <fstream>
#include <sstream>
#include "config.h"
#include <exception>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include "resource_manager.h"
#include "game/game.h"
#include "game/AnimationManager.h"
#include "tileCamera2D.h"

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
        {"tilemap", info.sTileMap},
        {"background", info.sBackground}
    };
}
void from_json(const json& json, GameLevelInfo& info)
{
    json.at("name").get_to(info.sName);
    json.at("difficulty").get_to(info.nDifficulty);
    json.at("completed").get_to(info.bCompleted);
    json.at("locked").get_to(info.bLocked);
    json.at("tilemap").get_to(info.sTileMap);
    json.at("background").get_to(info.sBackground);
}
void to_json(json& json, const std::vector<GameLevelInfo>& infos)
{
    for (int i = 0; i < int(infos.size()); i++)
        to_json(json[i], infos[i]);
}
void from_json(const json& json, std::vector<GameLevelInfo>& infos)
{
    int nLevel = 0;
    for (auto& i : json)
    {
        infos.push_back(i.get<GameLevelInfo>());
        infos[infos.size() - 1].nLevel = nLevel;
        nLevel++;
    }
}

// ************************
// GameLevel definitions
// ************************
void GameLevel::ProcessInput(InputInterface* input, float dt)
{
    pPlayer->ProcessKeyboard(input, dt);
}
void GameLevel::Update(float dt)
{
    TileCamera2D::Update(dt);
    this->PhysicsWorld->Update(dt, 5.0f);
    Map->Update(dt);
    pPlayer->Update(dt);
    pPlayer->SetSprite(pPlayer->Animator->GetSprite());
}
void GameLevel::Render(SpriteRenderer* pSpriteRenderer, TilemapRenderer* pTilemapRenderer)
{
    pSpriteRenderer->DrawSprite(*Background, glm::vec2(0.0f, 0.0f), Game::ScreenSize, 0.0f, glm::vec3(70 / 255.0f, 96 / 255.0f, 46 / 255.0f));
    pTilemapRenderer->AfterLayer_callback = [&](const Tmx::Map* map, const Tmx::Layer* layer, int nLayer) {
        if (layer->GetName() == "entity")
            pPlayer->Draw(pSpriteRenderer);
    };
    pTilemapRenderer->Draw(Map, glm::vec2(0.0f, 0.0f));
}
void GameLevel::Load(GameLevelInfo* pInfo)
{
    // Load level based on the info provided.
    this->Info = pInfo;
    // NOTE: Call order matters!!
    init_tilemap();
    init_physics_world();
    init_world_objects();
    init_background();
    init_player();
    init_tilecamera();
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
void GameLevel::init_background()
{
    Background = &ResourceManager::LoadTexture((ASSETS_DIR + Info->sBackground).c_str(), true, "background", Info->sName);
}
void GameLevel::init_world_objects()
{
    // ==== Load objects from tilemap ====
    Tmx::Map* map = Map->Map;
    // Load objects from tilesets and put them in correct locations in world.
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
    // Load objects from object group layers
    for (int nObjGroup = 0; nObjGroup < map->GetNumObjectGroups(); nObjGroup++)
    {
        const Tmx::ObjectGroup* pObjGroup = map->GetObjectGroup(nObjGroup);
        for (auto& object : pObjGroup->GetObjects())
        {
            if (object->GetName() == "start")
            {
                // Convert position from tilemap pixel-space to tile-space.
                vInitPlayerPosition = glm::vec2(float(object->GetX()) / map->GetTileWidth(), float(object->GetY()) / map->GetTileHeight());
            }
        }
    }
}
void GameLevel::init_player()
{
    AnimationManager* pAnim = ResourceManager::LoadAnimationManager(ASSETS_DIR "animations/PlayerAnimations_platformer.json", Info->sName);
    pPlayer = new Player(vInitPlayerPosition, glm::vec2(0.7f, 1.4f), pAnim->GetSprite(), glm::vec3(1.0f));
	pPlayer->SetRigidBody(Physics2D::RigidBody::CreateRectangleBody(pPlayer->Position, { 0.7f, 1.4f }, 5.0f, false, 0.0f));
	pPlayer->RBody->IsKinematic = true;
	pPlayer->RBody->Name = "player";
	pPlayer->RBody->Properties.Restitution = 0.0f;
	pPlayer->RBody->GravityScale = 1.0f;
	pPlayer->RBody->Properties.FrictionCoeff = 0.9f;
	pPlayer->IsJumping = false;
	pPlayer->Animator = pAnim;
    pPlayer->AddToWorld(PhysicsWorld);
}
void GameLevel::init_tilecamera()
{
    TileCamera2D::SetPosition(pPlayer->Position);
	TileCamera2D::SetRight(glm::vec2(1.0f, 0.0f));
	TileCamera2D::ScreenCoords = Game::ScreenSize;
	TileCamera2D::SetScale(glm::vec2(2.0f));
    TileCamera2D::SetFollow(pPlayer);
}

// Free all allocated resources for this level.
void GameLevel::Unload()
{
    // Check for null just in case some of the initializations failed.
    if (!(Map && PhysicsWorld && pPlayer))
        throw std::runtime_error("GameLevel::Unload(): Failed. Level is not initialized.");

    delete Map;
    delete PhysicsWorld;
    delete pPlayer;
    ResourceManager::DeleteGroup(Info->sName);
    Info = nullptr;
}