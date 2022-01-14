#include "game/TileSpace.h"
#include <algorithm>

std::vector<TileSpace*> TileSpace::Objects;

TileSpace::TileSpace()
{
    Objects.push_back(this);
}
TileSpace::~TileSpace()
{
    Objects.erase(
        std::remove_if(Objects.begin(), Objects.end(), [&](const TileSpace* obj){ return obj == this; }),
        Objects.end()
    );
}

void TileSpace::CallOnTileSizeChanged(glm::vec2& vNewTileSize) 
{ 
    for (auto& obj : TileSpace::Objects)
        obj->onTileSizeChanged(vNewTileSize); 
}