#pragma once
#include <glm/glm.hpp>

class ITileSpace
{
public:
    virtual void onTileSizeChanged(glm::vec2 newTileSize) = 0;
};