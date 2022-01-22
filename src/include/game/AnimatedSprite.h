#pragma once
#include "game/Sprite.h"
#include "interfaces/IAnimated.h"
#include <vector>
#include <functional>

struct SpriteFrame
{
    Texture2D texture;
    float life = 0.0f;       // Remaining life
    std::string path = "";


    SpriteFrame(Texture2D texture) : texture(texture), life(0.0f), path("") {}
    SpriteFrame() = default;
    SpriteFrame(const SpriteFrame& frame)
    {
        texture = frame.texture;
        life = frame.life;
        path = frame.path;
    }
};

class AnimatedSprite : public Sprite, public IAnimated
{
public:
    int Duration;
    std::vector<SpriteFrame> Frames;
    std::function<void (AnimatedSprite*)> onAnimationEnd;

    bool Spritesheet;
    int Sheet_spriteWidth;
    int Sheet_spriteHeight;
    int Sheet_offset;
    int Sheet_margin;
    std::string Sheet_source;

    AnimatedSprite(glm::vec2 position, glm::vec2 size, std::vector<Texture2D> frames, int frameDuration = 250);
    AnimatedSprite() = default;
    AnimatedSprite(const AnimatedSprite& sprite);

    void Update(float dt) override;
    void ResetAnimation();
    const std::vector<SpriteFrame>& GetFrames() const { return Frames; }
    const size_t& GetActiveFrame() const { return activeFrame; }
protected:    
    size_t activeFrame;

    void initFrames(std::vector<Texture2D> textures);
};
