#include "tilemap_renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "config.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <functional>
#include "game/game.h"


struct InstanceInfo
{	
	glm::vec2 Tex[4];	// 4 texture coordinates for 4 vertices
	float HasBackColor;
	glm::vec3 BackColor;
	glm::mat4 PVM;	// 4 * 4 array of floats
};
void getTextureCoordinates(float* tex, const Tmx::Tileset* set, int tile_id, bool flipV, bool flipH, bool flipD);

TilemapRenderer::TilemapRenderer(Shader shader)
	: shader(shader)	
	, VAO(0)
	, VBO(0)
	, AfterLayer_callback([=](const Tmx::Map* map, const Tmx::Layer* layer, int n_layer) {})
{
	initRenderData();
	this->shader.Use();		
	this->shader.SetInt("tilesetImage", 0);
}
TilemapRenderer::~TilemapRenderer()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &this->VBO_instance);
}
void TilemapRenderer::initRenderData()
{	
	float positions[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);	
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);		
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	glGenBuffers(1, &this->VBO_instance);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_instance);
	glm::ivec2 n_visibleTiles = TileCamera2D::GetNVisibleTiles();
	glBufferData(GL_ARRAY_BUFFER, ((n_visibleTiles.x + 4) * (n_visibleTiles.y + 4)) * sizeof(InstanceInfo), nullptr, GL_DYNAMIC_DRAW);
	GLsizei stride = sizeof(InstanceInfo);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)  0);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)( 4 * sizeof(float)));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)( 8 * sizeof(float)));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)(12 * sizeof(float)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (void*)(16 * sizeof(float)));
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)(20 * sizeof(float)));
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, stride, (void*)(24 * sizeof(float)));
	for (int i = 1; i <= 7; i++) {
		glEnableVertexAttribArray(i);
		glVertexAttribDivisor(i, 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
InstanceInfo initInstanceInfo()
{
	InstanceInfo i;
	memset((void*)(&i), 0, sizeof(InstanceInfo));
	return i;
}

void TilemapRenderer::Draw(Tilemap* tilemap, glm::vec2 pos)
{
	TileCamera2D::CurrentMapSize = glm::vec2((float)tilemap->Map->GetWidth(), (float)tilemap->Map->GetHeight());

	// Instance info for each Tileset texture ID.
	std::map<int, std::vector<InstanceInfo>> info;
	glm::vec2 scale = TileCamera2D::GetScale();

	Tmx::Map* map = tilemap->Map;
	for (int n_layer = 0; n_layer < map->GetNumTileLayers(); n_layer++)
	{
		const Tmx::TileLayer* layer = map->GetTileLayer(n_layer);
		if (!layer->IsVisible())		
			continue;

		info.clear();

		// Get offset for tiles.
		glm::vec2 offset = TileCamera2D::GetFirstVisibleTile();
		glm::ivec2 nVisibleTiles = TileCamera2D::GetNVisibleTiles();		
		glm::vec2 tileOffset = glm::vec2(offset.x - (int)offset.x, offset.y - (int)offset.y);		

		for (int x = -1; x <= nVisibleTiles.x + 1; x++)
		{
			for (int y = -1; y <= nVisibleTiles.y + 1; y++)
			{
				glm::ivec2 c_tile = glm::ivec2(x + (int)offset.x, y + (int)offset.y);
				int i_tileset = -1;
				if (c_tile.x >= 0 && c_tile.x < layer->GetWidth() && c_tile.y >= 0 && c_tile.y < layer->GetHeight())
					i_tileset = layer->GetTileTilesetIndex(c_tile.x, c_tile.y);

				// Check if tile is NOT empty
				if (i_tileset != -1)
				{
					const Tmx::Tileset* set = map->GetTileset(i_tileset);
					int tile_id = tilemap->GetTileId(layer, c_tile.x, c_tile.y);

					glm::mat4 model(1.0f);
					model = glm::translate(model, glm::vec3((pos + glm::vec2((float)x, (float)y) - tileOffset) * scale * Game::TileSize, 0.0f));
					if (set->GetTileHeight() > map->GetTileHeight())
						model = glm::translate(model, glm::vec3(0.0f, -scale.y * (set->GetTileHeight() - map->GetTileHeight()), 0.0f));
					model = glm::scale(model, glm::vec3(scale * Game::TileSize, 1.0f));

					InstanceInfo inf = initInstanceInfo();
					inf.PVM = this->Projection * TileCamera2D::GetViewMatrix() * model;
					getTextureCoordinates((float*)inf.Tex, set, tile_id,
						layer->IsTileFlippedVertically(c_tile.x, c_tile.y),
						layer->IsTileFlippedHorizontally(c_tile.x, c_tile.y),
						layer->IsTileFlippedDiagonally(c_tile.x, c_tile.y));
					info[tilemap->Tileset_textures[set].ID].push_back(inf);
				}	
				else if (n_layer == 0 && !map->GetBackgroundColor().IsTransparent())
				{
					glm::mat4 model(1.0f);
					glm::vec2 mapDimen(map->GetTileWidth(), map->GetTileHeight());
					model = glm::translate(model, glm::vec3((pos + glm::vec2(x, y) - tileOffset) * scale * Game::TileSize, 0.0f));
					model = glm::scale(model, glm::vec3(scale * Game::TileSize, 1.0f));

					Tmx::Color back_color = map->GetBackgroundColor();
					InstanceInfo inf = initInstanceInfo();
					inf.PVM = this->Projection * TileCamera2D::GetViewMatrix() * model;
					inf.HasBackColor = 1.0f;
					inf.BackColor = glm::vec3(back_color.GetRed() / 255.0f, back_color.GetGreen() / 255.0f, back_color.GetBlue() / 255.0f);
					info[0].push_back(inf);
				}
			}
		}

		this->shader.Use();
		glBindVertexArray(this->VAO);
		// Render all instances which sample from the same tileset texture.
		for(auto& i : info)
		{
			// Assign correct tileset texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, i.first);
			// Copy aligned instance info to GPU VBO
			glBindBuffer(GL_ARRAY_BUFFER, this->VBO_instance);
			glBufferSubData(GL_ARRAY_BUFFER, 0, i.second.size() * sizeof(InstanceInfo), i.second.data());
			// Render instanced array.
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)i.second.size());
		}

		AfterLayer_callback(map, layer, n_layer);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
// Swap texture coordinates.
void tex_swap_row(float* v, int a, int b)
{
	int f = a * 2;
	int s = b * 2;

	// Swap in chunks of 2 floats <-- vec2 <-- texture coordinates.
	float tmp[2];
	memcpy(  tmp, &v[f], 2 * sizeof(float));
	memcpy(&v[f], &v[s], 2 * sizeof(float));
	memcpy(&v[s],   tmp, 2 * sizeof(float));
}
void getTextureCoordinates(float* tex, const Tmx::Tileset* set, int tile_id, bool flipV, bool flipH, bool flipD)
{
	int tile_width = set->GetTileWidth();
	int tile_height = set->GetTileHeight();
	int margin = set->GetMargin();
	int spacing = set->GetSpacing();
	int columns = set->GetColumns();
	int tile_x = tile_id % columns;
	int tile_y = tile_id / columns;
	int xwm = tile_x * (tile_width + spacing) + margin;
	int yhm = tile_y * (tile_height + spacing) + margin;
	float set_width  = (float)set->GetImageWidth();
	float set_height = (float)set->GetImageHeight();

	tex[0] = xwm / set_width; 				 tex[1] = yhm / set_height;					// 0.0f, 0.0f
	tex[2] = xwm / set_width;		 		 tex[3] = (yhm + tile_height) / set_height;	// 0.0f, 1.0f
	tex[4] = (xwm + tile_width) / set_width; tex[5] = yhm / set_height;					// 1.0f, 0.0f
	tex[6] = (xwm + tile_width) / set_width; tex[7] = (yhm + tile_height) / set_height;	// 1.0f, 1.0f

	if (flipV) {	// Flip texture coordinates vertically.
		tex_swap_row(tex, 0, 1);
		tex_swap_row(tex, 2, 3);
	}
	if (flipH) {	// Flip texture coordinates horizontally.
		tex_swap_row(tex, 0, 2);
		tex_swap_row(tex, 1, 3);		
	}
	if (flipD) {	// Flip texture coordinates diagonally.
		tex_swap_row(tex, 0, 3);
	}
}
void TilemapRenderer::HandleScale(glm::vec2 scale)
{
	// Resize the buffer to match maximum visible tiles needed size.
	glm::ivec2 n_visibleTiles = TileCamera2D::GetNVisibleTiles();
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_instance);
	glBufferData(GL_ARRAY_BUFFER, ((n_visibleTiles.x + 4) * (n_visibleTiles.y + 4)) * sizeof(InstanceInfo), nullptr, GL_DYNAMIC_DRAW);
}