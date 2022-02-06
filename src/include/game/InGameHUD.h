#pragma once
#include "TextRenderer.h"
#include "sprite_renderer.h"
#include <vector>
#include "interfaces/Observer.h"
#include <cassert>
#include <queue>
#include "game/GameEvents.h"
#include "AtlasTextRenderer.hpp"
#include "Timer.hpp"

class GameLevel;

// Mappings to be used with SpriteRenderer partial sprite rendering.
// XY offset
// ZW size
struct HUD_mapping
{
    // Mapping for the HUD texture.
    glm::vec4 vHeartFull = glm::vec4(0.0f, 0.0f, 16.0f, 16.0f);
    glm::vec4 vHeartEmpty = glm::vec4(16.0f, 0.0f, 16.0f, 16.0f);
    int nHeartSpacing = 2;
    glm::vec4 vCoin = glm::vec4(0.0f, 16.0f, 9.0f, 9.0f);

    // Actual position and size on the screen.
    glm::vec4 vHeartsScreen = glm::vec4(0.0f, 0.0f, 16.0f, 16.0f);
    glm::vec2 fHeartsPadding = glm::vec2(5.0f, 5.0f);
    glm::vec4 vCoinScreen = glm::vec4(0.0f, 16.0f + fHeartsPadding.y, 9.0f, 9.0f);

    // Text sizes.
    glm::vec2 vCoinText = glm::vec2(7.0f, 7.0f);
    glm::vec2 vLevelText = glm::vec2(14.0f, 14.0f);
    glm::vec2 vElapsedTimeText = glm::vec2(12.0f, 12.0f);
};

class InGameHUD : public IObserver
{
public:
    GameLevel* pLevel;
    Texture2D HUDTexture;
    glm::vec2 vScale = glm::vec2(3.0f);
    // HUD padding.
    glm::vec2 vPadding = glm::vec2(10.0f, 10.0f);
    glm::vec2 vFontSize;
    
    // Level **MUST** be loaded.
    InGameHUD(GameLevel* pLevel, Texture2D texHUD, AtlasTextRenderer* pTextRenderer);
    ~InGameHUD();

    void Update(float dt);
    void Render(SpriteRenderer* pSpriteRenderer);

    // Observer implementation.
    void OnNotify(IObserverSubject* sender, int message, void* args = nullptr);

protected:
    const HUD_mapping mapping;
    std::queue<Event> events;
    float fTextScale = 1.0f;
    AtlasTextRenderer* pTextRenderer;
    int skip_heart = -1;

    void handle_events();
    void render_coins(SpriteRenderer* pSpriteRenderer);
    void render_hp(SpriteRenderer* pSpriteRenderer);
    void render_time(SpriteRenderer* pSpriteRenderer);
    void render_level_name(SpriteRenderer* pSpriteRenderer);
    glm::vec2 get_heart_position(int nHeart);

    inline glm::vec2 get_offset(const glm::vec4& vec) { return glm::vec2(vec.x, vec.y); }
    inline glm::vec2 get_size(const glm::vec4& vec) { return glm::vec2(vec.z, vec.w); }
    inline glm::vec2 get_text_scale(const glm::vec2& wanted_size) { return wanted_size / vFontSize; }
};
