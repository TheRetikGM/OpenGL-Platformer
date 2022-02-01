#pragma once
#include "TextRenderer.h"
#include "sprite_renderer.h"
#include <vector>
#include "GameLevel.h"

class InGameHUD
{
public:
    GameLevel* pLevel;
    
    // Level **MUST** be loaded.
    InGameHUD(GameLevel* pLevel, Texture2D texHUD);

    void Update(float dt);
    void Render();

protected:

    void renderCoins();
    void renderHP();
    void renderTime();
    void renderLevelName();
};