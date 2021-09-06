#include "game/AnimatedSprite.h"
#include <algorithm>

AnimatedSprite::AnimatedSprite(glm::vec2 position, glm::vec2 size, std::vector<Texture2D> frames, int frameDuration)
    : Sprite(position, size, frames[0])
    , Frames()
    , Duration(frameDuration)
    , onAnimationEnd([&](AnimatedSprite *){})
    , Spritesheet(false)
    , Sheet_spriteWidth(0)
    , Sheet_spriteHeight(0)
    , Sheet_margin(0)
    , Sheet_offset(0)
    , Sheet_source("")
{    
    initFrames(frames);
}
AnimatedSprite::AnimatedSprite(const AnimatedSprite& sprite)
{
    this->Position = sprite.Position;
    this->Color = sprite.Color;
    this->Duration = sprite.Duration;
    this->Rotation = sprite.Rotation;
    this->Size = sprite.Size;
    this->Texture = sprite.Texture;    
    for (size_t i = 0; i < sprite.Frames.size(); i++)
    {
        SpriteFrame f = sprite.Frames[i];
        Frames.push_back(f);
    }
    this->activeFrame = sprite.activeFrame;
    this->onAnimationEnd = sprite.onAnimationEnd;
    this->Spritesheet = sprite.Spritesheet;
    this->Sheet_margin = sprite.Sheet_margin;
    this->Sheet_offset = sprite.Sheet_offset;
    this->Sheet_source = sprite.Sheet_source;
    this->Sheet_spriteWidth = sprite.Sheet_spriteWidth;
    this->Sheet_spriteHeight = sprite.Sheet_spriteHeight;
    this->FlipTex_x = sprite.FlipTex_x;
    this->FlipTex_y = sprite.FlipTex_y;
}

void AnimatedSprite::initFrames(std::vector<Texture2D> textures)
{
    std::for_each(textures.begin(), textures.end(), [&](Texture2D& tex){
        SpriteFrame frame(tex);
        this->Frames.push_back(frame);
    });
    this->activeFrame = 0;
    this->Frames[0].life = this->Duration / 1000.0f;
}

void AnimatedSprite::Update(float dt)
{
    SpriteFrame& currentFrame = Frames[activeFrame];
    currentFrame.life -= dt;

    if (currentFrame.life <= 0.0f)
    {
        activeFrame = (activeFrame + 1) % Frames.size();
        Frames[activeFrame].life = Duration / 1000.0f;
        this->Texture = Frames[activeFrame].texture;

        if (activeFrame == 0)
            onAnimationEnd(this);
    }
}
void AnimatedSprite::ResetAnimation()
{
    this->activeFrame = 0;
    this->Frames[0].life = this->Duration / 1000.0f;
}