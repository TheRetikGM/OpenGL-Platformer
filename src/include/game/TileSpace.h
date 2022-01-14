#pragma once
#include <glm/glm.hpp>
#include <vector>

class TileSpace
{
public:
    static std::vector<TileSpace*> Objects;

    TileSpace();
    ~TileSpace();

    static void CallOnTileSizeChanged(glm::vec2& vNewTileSize);

    virtual void onTileSizeChanged(glm::vec2 newTileSize) {}
};