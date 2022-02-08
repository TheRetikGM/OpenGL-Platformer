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
#include "game/GameEvents.h"
#include "Helper.hpp"

// Define square root just in case that some platform does not have it. (ig. Windows..)
#ifndef M_SQRT1_2
    // 1/2 * sqrt(2)
    #define M_SQRT1_2 0.707106781186547524401
#endif

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
        {"background", info.sBackground},
        {"single_animations", info.sSingleAnimationsPath},
        {"lives", info.nLives}
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
    json.at("single_animations").get_to(info.sSingleAnimationsPath);
    json.at("lives").get_to(info.nLives);
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
void GameLevel::OnNotify(IObserverSubject* obj, int message, void* args)
{
    if (std::find(acceptedMessages.begin(), acceptedMessages.end(), message) != acceptedMessages.end())
        eventQueue.emplace(Event{ obj, message, args });
    notify(message, args);
}
void GameLevel::handle_events(float dt)
{
    while (!eventQueue.empty())
    {
        Event e = eventQueue.front();
        eventQueue.pop();
        switch (e.message)
        {
        case PLAYER_LOST_LIFE:
            if (pPlayer->Lives == 0)
                OnPlayerDied();
            else
                pPlayer->Animator->PlayOnce("hit", "", true);
            break;
        case PLAYER_JUMPED:
            pSingleAnimations->Play("before_jump", "", glm::vec2(pPlayer->Position.x - 0.5f, int(pPlayer->Position.y)), glm::vec2(1.0f, 1.0f), true);
            break;
        case PLAYER_LANDED:
            pPlayer->Animator->PlayOnce("after_jump");
            pSingleAnimations->Play("after_jump", "", glm::vec2(pPlayer->Position.x - 0.5f, int(pPlayer->Position.y)), glm::vec2(1.0f, 1.0f), true);
            break;
        case PLAYER_COLLIDE_COIN:
            pickup_coin((Physics2D::RigidBody*)e.args);
            break;
        case PLAYER_WALL_JUMPED:
            pPlayer->Animator->PlayOnce("double_jump");
            break;
        case PLAYER_REACHED_FINISH:
            State = InGameState::paused_dialog;
            break;
        default:
            break;
        }
    }
}
void GameLevel::OnPlayerDied()
{
    pPlayer->Animator->PlayOnce("die", "", true);
    notify(PLAYER_DIED);
}
void GameLevel::pickup_coin(Physics2D::RigidBody* coin)
{
    if (coins.count(coin) == 0)
        return;
    MapTileInfo inf = coins[coin];
    if (!Map->IsHidden(inf))
    {
        coins.erase(coin);
        Map->HideTile(inf);
        pSingleAnimations->Play("pickup_coin", "", coin->GetPosition() - glm::vec2(0.0f, 1.0f), glm::vec2(0.5f, 1.0f), true);
        PhysicsWorld->RemoveBody(PhysicsWorld->GetBodyIndex(coin));
        nCoins++;
    }
}
void GameLevel::ProcessInput(InputInterface* input, float dt)
{
    if (State == InGameState::running)
        pPlayer->ProcessKeyboard(input, dt);

    if (input->Pressed(GLFW_KEY_P))
        State = (State == InGameState::running) ? InGameState::paused_dialog : InGameState::running;
}
void GameLevel::Update(float dt)
{
    if (State == InGameState::running)
    {
        handle_events(dt);
        TileCamera2D::Update(dt);
        this->PhysicsWorld->Update(dt, 5.0f);
        Map->Update(dt);
        pPlayer->Update(dt);
        pPlayer->SetSprite(pPlayer->Animator->GetSprite());
        pSingleAnimations->Update(dt);
        pHUD->Update(dt);
        fElapsedTime += dt;
    }
}
void GameLevel::Render(SpriteRenderer* pSpriteRenderer, TilemapRenderer* pTilemapRenderer)
{
    pSpriteRenderer->ForceColor(true).DrawSprite(*Background, glm::vec2(0.0f, 0.0f), Game::ScreenSize, 0.0f, glm::vec3(70 / 255.0f, 96 / 255.0f, 46 / 255.0f)).ForceColor(false);
    pTilemapRenderer->AfterLayer_callback = [&](const Tmx::Map* map, const Tmx::Layer* layer, int nLayer) {
        if (layer->GetName() == "entity")
            pPlayer->Draw(pSpriteRenderer);
    };
    pTilemapRenderer->Draw(Map, glm::vec2(0.0f, 0.0f));
    pSingleAnimations->Render(pSpriteRenderer);
    pHUD->Render(pSpriteRenderer);
    
    if (State == InGameState::paused_dialog)
    {
        mForms["won"]->Render(pSpriteRenderer);
    }
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
    init_single_animations();
    init_hud();
    init_forms();
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

// Sort polygon vertices in clockwise order.
void sort_polygon(const glm::vec2& poly_center, std::vector<glm::vec2>& points)
{
    struct polar_coord { glm::vec2 orig; float r; float phi; };
    std::vector<polar_coord> points_polar;
    points_polar.reserve(points.size());

    // Convert Cartesian to polar. x --> r, y --> phi
    for (auto& point : points)
    {
        point -= poly_center;
        float r = std::sqrt(point.x * point.x + point.y * point.y);
        float phi = std::atan2(point.y, point.x);
        if (phi < 0.0f)
            phi = M_PI * 2.0f + phi;
        points_polar.push_back(polar_coord{ point, r, phi });
    }

    // Sort in ascending order. 
    // Note: our y axis is pointing down, so we sort in ascending order, because the angle "points" down too.
    // std::sort(points.begin(), points.end(), [](const glm::vec2& a, const glm::vec2& b) { return a.y < b.y; });
    std::sort(points_polar.begin(), points_polar.end(), [](auto a, auto b) { return a.phi < b.phi; });

    // Convert polar back to Cartesian.
    for (size_t i = 0; i < points_polar.size(); i++)
    {
        points[i] = points_polar[i].orig + poly_center;
    }
}
void GameLevel::init_world_objects()
{
    // ==== Load objects from tilemap ====
    Tmx::Map* map = Map->Map;
    glm::vec2 map_tile_size = glm::vec2(map->GetTileWidth(), map->GetTileHeight());
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
            const Tmx::MapTile& map_tile = layer->GetTile(x, y);
            if (tile && tile->HasObjects())
            {
                for (Tmx::Object* obj : tile->GetObjects())
                {
                    const Tmx::Polygon* polygon = obj->GetPolygon();
                    const Tmx::Ellipse* ellipse = obj->GetEllipse();
                    glm::vec2 set_tile_size = glm::vec2(set->GetTileWidth(), set->GetTileHeight());
					glm::vec2 tilespace_pos = glm::vec2(x, y);
                    glm::vec2 local_offset = glm::vec2(obj->GetX(), obj->GetY()) / set_tile_size;
					glm::vec2 local_size = glm::vec2(obj->GetWidth(), obj->GetHeight()) / set_tile_size;
					Physics2D::RigidBody* body = nullptr;

                    if (polygon)
                    {
                        // Convert polygon points into range 0..1 of tile
                        std::vector<glm::vec2> points;
                        for (int nPoint = 0; nPoint < polygon->GetNumPoints(); nPoint++)
                        {
                            const Tmx::Point point = polygon->GetPoint(nPoint);
                            points.push_back(glm::vec2(point.x, point.y) / set_tile_size + local_offset);
                        }
                        
                        // Rotate tile as needed.
                        glm::vec2 polygon_center = Physics2D::GetPolygonCenter(points);
                        polygon_center -= glm::vec2(0.5f);

                        for (size_t nPoint = 0; nPoint < points.size(); nPoint++)
                        {
                            glm::vec2& point = points[nPoint];
                            point -= glm::vec2(0.5f);   // Translate to tile center. As we are flipping over the tile center.

                            if (map_tile.flippedHorizontally) {
                                if (nPoint == 0)
                                    polygon_center.x = -polygon_center.x;
                                point.x = -point.x;
                            }
                            if (map_tile.flippedVertically) {
                                if (nPoint == 0)
                                    polygon_center.y = -polygon_center.y;
                                point.y = -point.y;
                            }
                            if (map_tile.flippedDiagonally)
                            {
                                const auto flip_diag = [](glm::vec2& p) {
                                    p = glm::vec2(M_SQRT1_2 * (p.x - p.y), -M_SQRT1_2 * (p.x + p.y));
                                    p.x = -p.x;
                                    p = glm::vec2(-M_SQRT1_2 * (p.x - p.y), M_SQRT1_2 * (p.x + p.y));
                                };
                                if (nPoint == 0)
                                    flip_diag(polygon_center);
                                flip_diag(point);
                            }
                            
                            point += glm::vec2(0.5f); // Translate back.
                        }
                        polygon_center += glm::vec2(0.5f);
                        sort_polygon(polygon_center, points);

                        polygon_center = Physics2D::GetPolygonCenter(points);

                        tilespace_pos += polygon_center;  // Polygon has position in the center.
                        body = this->PhysicsWorld->AddPolygonBody(tilespace_pos, points, 2.0f, true, 1.0f);
					}
                    else if (ellipse)
                    {
                        glm::vec2 size_ratio(1.0f);
                        if (set_tile_size != map_tile_size)
                        {
                            size_ratio = set_tile_size / map_tile_size;
                            glm::vec2 rel_size = glm::vec2(0.0f, 1.0f - size_ratio.y);
                            tilespace_pos += rel_size;
                        }

                        float r_local = ((ellipse->GetRadiusX() + ellipse->GetRadiusY()) * 0.5f) / set_tile_size.x;
                        r_local *= size_ratio.x;
                        local_offset *= size_ratio;
                        tilespace_pos += local_offset + glm::vec2(r_local);
                        body = this->PhysicsWorld->AddCircleBody(tilespace_pos, r_local, 2.0f, true, 1.0f);
                    }
					else {
						body = this->PhysicsWorld->AddRectangleBody(tilespace_pos + local_offset, local_size, 2.0f, true, 1.0f);
					}

                    // Assign body the additional properties set on the collider.
					if (body)
						body->Name = obj->GetProperties().GetStringProperty("name");
                    if (body->Name == "coin")
                    {
                        nCoinsTotal++;
                        coins[body] = MapTileInfo(layer->GetName(), x, y);
                    }
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
            else if (object->GetName() == "finish")
            {
                glm::vec2 vSizeInTiles = glm::vec2(object->GetWidth(), object->GetHeight()) / map_tile_size;
                glm::vec2 vPosInTiles = glm::vec2(object->GetX(), object->GetY()) / map_tile_size;
                this->PhysicsWorld->AddRectangleBody(vPosInTiles, vSizeInTiles, 2.0f, true, 1.0, true)->Name = "finish";
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
    pPlayer->AddObserver(this);
    pPlayer->Size = { 0.7f, 1.4f };
    pPlayer->Lives = Info->nLives;
}
void GameLevel::init_tilecamera()
{
    TileCamera2D::SetPosition(pPlayer->Position);
	TileCamera2D::SetRight(glm::vec2(1.0f, 0.0f));
	TileCamera2D::ScreenCoords = Game::ScreenSize;
	TileCamera2D::SetScale(glm::vec2(2.0f));
    TileCamera2D::SetFollow(pPlayer);
}
void GameLevel::init_single_animations()
{
    pSingleAnimations = new SingleAnimations(ASSETS_DIR + Info->sSingleAnimationsPath, "level_" + std::to_string(Info->nLevel) + "_single_animations");
}
void GameLevel::init_hud()
{
    Texture2D& tex = ResourceManager::LoadTexture(ASSETS_DIR "sprites/HUD.png", true, "hud_texture", Info->sName);
    tex.SetMagFilter(GL_NEAREST).SetMinFilter(GL_NEAREST).UpdateParameters();

    Texture2D& font = ResourceManager::LoadTexture(ASSETS_DIR "fonts/atlas.png", true, "font_atlas", Info->sName);
    font.SetMagFilter(GL_NEAREST).SetMinFilter(GL_NEAREST).UpdateParameters();
    pTextRenderer = new AtlasTextRenderer();
    pTextRenderer->Load(font, glm::vec2(7.0f));

    pHUD = new InGameHUD(this, tex, pTextRenderer);
}
void GameLevel::init_forms()
{
    // You won form initialization.
    mForms["won"] = std::make_shared<Forms::Form>(pTextRenderer);
    mForms["won"]->AddLabel("lblWon", "You won!", glm::vec2(64.0f), Helper::HexToRGB(0xE0BA1E));

	auto label1 = new Forms::Label("Coins", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f, 1.0f, 0.0f), pTextRenderer);
	auto label2 = new Forms::Label("27/40", glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(1.0f, 1.0f, 1.0f), pTextRenderer);
	mForms["won"]->AddPair("pairCoins", std::shared_ptr<Forms::Control>(label1), std::shared_ptr<Forms::Control>(label2));

    label1 = new Forms::Label("Time", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(0.0f, 0.7f, 0.08f), pTextRenderer);
	label2 = new Forms::Label("230s", glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(1.0f, 1.0f, 1.0f), pTextRenderer);
	mForms["won"]->AddPair("pairTime", std::shared_ptr<Forms::Control>(label1), std::shared_ptr<Forms::Control>(label2));

    label1 = new Forms::Label("Press", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), pTextRenderer);
	label2 = new Forms::Label("r", glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(0.0f, 1.0f, 0.0f), pTextRenderer);
    auto label3 = new Forms::Label("to restart", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), pTextRenderer);
	mForms["won"]->AddControl("rowInputHelper", std::make_shared<Forms::Row>(glm::vec2(0.0f, 0.0f)), Forms::ControlType::unknown);
    mForms["won"]->GetControl<Forms::Row*>("rowInputHelper")->AddControl("lblTextP1", std::shared_ptr<Forms::Control>(label1), Forms::ControlType::label);
    mForms["won"]->GetControl<Forms::Row*>("rowInputHelper")->AddControl("lblTextP2", std::shared_ptr<Forms::Control>(label2), Forms::ControlType::label);
    mForms["won"]->GetControl<Forms::Row*>("rowInputHelper")->AddControl("lblTextP3", std::shared_ptr<Forms::Control>(label3), Forms::ControlType::label);

    mForms["won"]->AddLabel("lblAnyKey", "Press any key to continue", glm::vec2(24.0f), glm::vec3(0.1f, 0.9f, 0.8f));
    mForms["won"]->SetGravity(Forms::Gravity::center);
    mForms["won"]->MoveTo((Game::ScreenSize - mForms["won"]->vSize) * 0.5f);
}

void GameLevel::OnResize()
{
    if (mForms.count("won") != 0)
        mForms["won"]->MoveTo((Game::ScreenSize - mForms["won"]->vSize) * 0.5f);
}

// Free all allocated resources for this level.
void GameLevel::Unload()
{
    // Check for null just in case some of the initializations failed.
    if (!(Map && PhysicsWorld && pPlayer))
        return;

    delete pHUD;
    delete Map;
    delete PhysicsWorld;
    delete pPlayer;
    delete pSingleAnimations;
    delete pTextRenderer;
    mForms.clear();
    ResourceManager::DeleteGroup(Info->sName);
    Info = nullptr;
}