#pragma once

#include "Tmx.h"
#include "texture.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "shader.h"
#include "tilemap.h"
#include "camera/tileCamera2D.h"
#include <functional>


class TilemapRenderer
{
public:
	glm::mat4 Projection;
	std::function<void (const Tmx::Map*, const Tmx::Layer*, int)> AfterLayer_callback;

	TilemapRenderer(Shader shader);
	~TilemapRenderer();

	void Draw(Tilemap* tilemap, glm::vec2 pos_offset);
	void HandleScale(glm::vec2 scale);
private:	
	Shader shader;
	unsigned int VAO, VBO, VBO_instance;


	void initRenderData();	
	void updateVBO(const Tmx::Tileset* set, int tile_id, bool flipV, bool flipH, bool flipD);
};