#include "game/InGameHUD.h"
#include "game/GameLevel.h"
#include "game/game.h"

InGameHUD::InGameHUD(GameLevel* pLevel, Texture2D texHUD, AtlasTextRenderer* pTextRenderer) 
    : pLevel(pLevel)
    , HUDTexture(texHUD)
    , pTextRenderer(pTextRenderer)
    , vFontSize(pTextRenderer->vCharSize)
{
    if (!pLevel->Info)
        throw std::invalid_argument("InGameHUD::InGameHUD(): pLevel->Info == nullptr. Did you forget to load the level first?");
    pLevel->AddObserver(this);
}
InGameHUD::~InGameHUD()
{
    pLevel->RemoveObserver(this);
}

void InGameHUD::OnNotify(IObserverSubject* sender, int message, std::any args)
{
    events.emplace(Event { sender, message, args });
}

void InGameHUD::Update(float dt)
{
    handle_events();
}
void InGameHUD::Render(SpriteRenderer* pSpriteRenderer)
{
    render_hp(pSpriteRenderer);
    render_coins(pSpriteRenderer);
    render_time(pSpriteRenderer);
    render_level_name(pSpriteRenderer);
}
void InGameHUD::render_coins(SpriteRenderer* pSpriteRenderer)
{
    glm::vec2 offset = get_heart_position(0) + glm::vec2(0.0f, mapping.vHeartsScreen.w * vScale.y);

    pSpriteRenderer->DrawPartialSprite(
        HUDTexture,
        get_offset(mapping.vCoin),
        get_size(mapping.vCoin),
        offset + get_offset(mapping.vCoinScreen),
        get_size(mapping.vCoinScreen) * vScale
    );

    std::string coins = std::to_string(pLevel->nCoins) + "/" + std::to_string(pLevel->nCoinsTotal);
    glm::vec2 text_scale = get_text_scale(mapping.vCoinText);

    pTextRenderer->RenderText(
        pSpriteRenderer, 
        coins,
        offset + (get_offset(mapping.vCoinScreen) + glm::vec2(mapping.vCoinScreen.z + 2, 1.0f) * vScale),
        text_scale * vScale
    );
}
void InGameHUD::render_hp(SpriteRenderer* pSpriteRenderer)
{
    int lives_total = pLevel->Info->nLives;
    int lives_current = pLevel->pPlayer->Lives;

    glm::vec2 size = get_size(mapping.vHeartsScreen);
    for (int i = 0; i < lives_total; i++)
    {
        if (i == skip_heart)
            continue;

        glm::vec4 heart = (i < lives_current) ? mapping.vHeartFull : mapping.vHeartEmpty;
        glm::vec2 part_offset = get_offset(heart);
        glm::vec2 part_size = get_size(heart);

        pSpriteRenderer->DrawPartialSprite(
            HUDTexture, part_offset, part_size,
            get_heart_position(i),
            size * vScale
        );
    }
}
void InGameHUD::render_time(SpriteRenderer* pSpriteRenderer)
{
    glm::vec2 text_scale = get_text_scale(mapping.vElapsedTimeText) * vScale;

    std::string sTime = std::to_string(int(pLevel->fElapsedTime));

    glm::vec2 pos(0.0f);
    pos.x = Game::ScreenSize.x - pTextRenderer->GetStringSize(sTime, text_scale).x - vPadding.x;
    pos.y = vPadding.y;

    pTextRenderer->RenderText(
        pSpriteRenderer,
        sTime, pos, 
        text_scale
    );
}
void InGameHUD::render_level_name(SpriteRenderer* pSpriteRenderer)
{

}

glm::vec2 InGameHUD::get_heart_position(int nHeart)
{
    glm::vec2 pos = vPadding + get_offset(mapping.vHeartsScreen);
    pos.x += nHeart * (mapping.vHeartsScreen.z + mapping.nHeartSpacing) * vScale.x;
    return pos;
}

void InGameHUD::handle_events()
{
    if (events.empty())
        return;

    Event e = events.front();
    events.pop();
    
    switch (e.message)
    {
    case PLAYER_LOST_LIFE:
        pLevel->pSingleAnimations->Play(
            "heart_pop", "", 
            get_heart_position(pLevel->pPlayer->Lives),
            get_size(mapping.vHeartsScreen) * vScale,
            false, // tile space
            true,   // one time
            [&](SpriteAnimation* a) { skip_heart = -1; }
        );
        skip_heart = pLevel->pPlayer->Lives;
        break;
    default:
        break;
    }
}