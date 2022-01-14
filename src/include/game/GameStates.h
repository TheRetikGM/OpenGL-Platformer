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
    virtual void ProcessInput(int* keys, int* keys_processed, float dt) = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
    virtual void OnEnter() {};
    virtual void OnExit() {};
};

class DrawLevelState : public IGameState
{
public:
    TilemapRenderer*    pTilemapRenderer = nullptr;
    SpriteRenderer*     pSpriteRenderer = nullptr;
    Player*             pPlayer = nullptr;
    GameLevel*          pLevel = nullptr;

    virtual void Init();
    virtual void ProcessInput(int* keys, int* keys_processed, float dt) {}
    virtual void Update(float dt) {}
    virtual void Render();
    virtual void OnExit();

    void OnLayerRendered(const Tmx::Map *map, const Tmx::Layer *layer, int n_layer);
};
class ActiveLevelState : public DrawLevelState
{
public:
    void Init() {}
    void ProcessInput(int* keys, int* keys_processed, float dt) {}
    void Update(float dt) {}
    void Render() {}
};
class CompletedLevelState : public DrawLevelState
{
public:
    void Init() {}
    void ProcessInput(int* keys, int* keys_processed, float dt) {}
    void Update(float dt) {}
    void Render() {}
};

class MenuState : public IGameState
{
public:
    void Init() {}
    void ProcessInput(int* keys, int* keys_processed, float dt) {}
    void Update(float dt) {}
    void Render() {}
};
class MainMenuState : public MenuState
{
public:
    void Init() {}
    void ProcessInput(int* keys, int* keys_processed, float dt) {}
    void Update(float dt) {}
    void Render() {}
};
class PauseMenuState : public MenuState, public DrawLevelState
{
public:
    void Init() {}
    void ProcessInput(int* keys, int* keys_processed, float dt) {}
    void Update(float dt) {}
    void Render() {}
};