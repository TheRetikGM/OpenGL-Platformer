#pragma once
#include "tilemap_renderer.h"
#include "sprite_renderer.h"
#include "TextRenderer.h"
#include "game/Player.h"
#include "game/GameLevelsManager.h"
#include "AnimationManager.h"

// Base game state interface.
class IGameState
{
public:
    virtual void Init() = 0;
    virtual IGameState* ProcessInput(bool* keys, bool* keys_processed, float dt) = 0;
    virtual IGameState* Update(float dt) = 0;
    virtual void Render() = 0;
    virtual void OnEnter() {};
    virtual void OnExit() {};
};

class InLevelState : public IGameState
{
public:
    TilemapRenderer*    pTilemapRenderer = nullptr;
    SpriteRenderer*     pSpriteRenderer = nullptr;
    Player*             pPlayer = nullptr;
    GameLevel*          pLevel = nullptr;

    virtual void Init();
    virtual IGameState* ProcessInput(bool* keys, bool* keys_processed, float dt) { return nullptr; }
    virtual IGameState* Update(float dt) { return nullptr; }
    virtual void Render();
    virtual void OnExit();

    void OnLayerRendered(const Tmx::Map *map, const Tmx::Layer *layer, int n_layer);
};
class ActiveLevelState : public InLevelState
{
public:
    void Init();
    IGameState* ProcessInput(bool* keys, bool* keys_processed, float dt);
    IGameState* Update(float dt);
    // void Render();
};
class CompletedLevelState : public InLevelState
{
public:
    void Init() {}
    IGameState* ProcessInput(bool* keys, bool* keys_processed, float dt) { return nullptr; }
    IGameState* Update(float dt) { return nullptr; }
    void Render() {}
};

class MenuState : public IGameState
{
public:
    void Init() {}
    IGameState* ProcessInput(bool* keys, bool* keys_processed, float dt) { return nullptr; }
    IGameState* Update(float dt) { return nullptr; }
    void Render() {}
};
class MainMenuState : public MenuState
{
public:
    void Init() {}
    IGameState* ProcessInput(bool* keys, bool* keys_processed, float dt) { return nullptr; }
    IGameState* Update(float dt) { return nullptr; }
    void Render() {}
};
class PauseMenuState : public MenuState, public InLevelState
{
public:
    void Init() {}
    IGameState* ProcessInput(bool* keys, bool* keys_processed, float dt) { return nullptr; }
    IGameState* Update(float dt) { return nullptr; }
    void Render() {}
};