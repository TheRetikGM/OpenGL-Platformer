#include "tilemap.h"
#include <exception>
#include <stdexcept>
#include <algorithm>
#include "resource_manager.h"
#include "config.h"
#include "stb_image.h"

Tilemap::Tilemap(const char* path)
	: Map(nullptr)
	, AnimTiles()
	, Tileset_textures()
	, directory("")
{
	std::string s(path);
	directory = s.substr(0, s.find_last_of('/'));

	loadTilemap(path);
	loadTextures();
	initAnimatedTiles();
}
Tilemap::~Tilemap()
{
	for (auto& i : AnimTiles)
		delete i.second;
	AnimTiles.clear();
	// Textures will get deleted by ResourceManager class. -- NOT ACTUAL
	for (auto& i : Tileset_textures)
		glDeleteTextures(1, &i.second.ID);	
	Tileset_textures.clear();
	delete Map;	
}
void Tilemap::loadTilemap(const char* path)
{
	// Load map and check for errors.
	Map = new Tmx::Map();
	Map->ParseFile(path);
	if (Map->HasError())
		throw std::runtime_error(Map->GetErrorText());

	this->loadTextures();
}
unsigned char* getTextureData(std::string filename, int tile_width, int tile_height, int margin, int spacing, int& new_width, int& new_height, int& n_channels)
{
	// Load image.
	int img_width, img_height;
	unsigned char* data = stbi_load(filename.c_str(), &img_width, &img_height, &n_channels, 0);

	if (!data)
		throw std::runtime_error(("Tilemap::getTextureData(): Failed to load " + filename).c_str());

	// Allocate storage for new image with required spacing.	
	int tiles_x = (img_width - margin / 2) / (tile_width + spacing);
	int tiles_y = (img_height - margin / 2) / (tile_height + spacing);	
	new_width = (tiles_x - 1) * (tile_width + 2) + 2 + tile_width;
	new_height = (tiles_y - 1) * (tile_height + 2) + 2 + tile_height;
	int size = new_width * new_height * n_channels;
	unsigned char* data_converted = new unsigned char[size];
	memset(data_converted, 0, size * sizeof(unsigned char));

	// Helper lambda functions.
	auto GetTexel = [&](int x, int y) {
		return &data[(y * img_width + x) * n_channels];
	};
	auto GetTexel_new = [&](int x, int y) {
		return &data_converted[(y * new_width + x) * n_channels];
	};
	auto CopyTexel_new = [&](int srcX, int srcY, int dstX, int dstY) {
		memcpy(GetTexel_new(dstX, dstY), GetTexel_new(srcX, srcY), n_channels * sizeof(unsigned char));
	};
	auto CopyTile = [&](int t_x, int t_y, int offset_x, int offset_y) { 
		int tile_pixel_x = margin + (tile_width + spacing) * t_x;
		int tile_pixel_y = margin + (tile_height + spacing) * t_y;		
		for (int y = 0; y < tile_height; y++)
		{
			unsigned char* tile_ptr = GetTexel(tile_pixel_x, tile_pixel_y + y);
			unsigned char* tile_new_ptr = GetTexel_new(offset_x, offset_y + y);
			memcpy(tile_new_ptr, tile_ptr, (size_t)tile_width * (size_t)n_channels * sizeof(unsigned char));	// Copy whole row of tile pixels.
			CopyTexel_new(offset_x, offset_y + y, offset_x - 1, offset_y + y);
			CopyTexel_new(offset_x + tile_width - 1, offset_y + y, offset_x + tile_width, offset_y + y);
			if (y == 0)
			{
				unsigned char* upper_row_ptr = GetTexel_new(offset_x, offset_y - 1);
				memcpy(upper_row_ptr, tile_ptr, (size_t)tile_width * (size_t)n_channels * sizeof(unsigned char));
				CopyTexel_new(offset_x, offset_y, offset_x - 1, offset_y - 1);		// Copy upper-left corner.
				CopyTexel_new(offset_x + tile_width - 1, offset_y, offset_x + tile_width, offset_y - 1); 	// Copy upper-right corner.
			}
			else if (y == tile_height - 1)
			{
				unsigned char* under_row_ptr = GetTexel_new(offset_x, offset_y + tile_height);
				memcpy(under_row_ptr, tile_ptr, (size_t)tile_width * (size_t)n_channels * sizeof(unsigned char));
				CopyTexel_new(offset_x, offset_y + tile_height - 1, offset_x - 1, offset_y + tile_height);		// Copy down-left corner.
				CopyTexel_new(offset_x + tile_width - 1, offset_y + tile_height - 1, offset_x + tile_width, offset_y + tile_height);	// Copy down-right corner.
			}
		}
	};
	
	// Set all alpha values of new data to 1
	if (n_channels == 4)
		for (int i = 0; i < new_width * new_height; i++)
			GetTexel_new(i % new_width, i / new_width)[3] = (unsigned char)(255);

	// Copy tiles from one image to another and offset them.
	for (int x = 0; x < tiles_x; x++)
		for (int y = 0; y < tiles_y; y++)
			CopyTile(x, y, 1 + (tile_width + 2) * x, 1 + (tile_height + 2) * y);
	
	stbi_image_free(data);
	return data_converted;
}
void Tilemap::loadTextures()
{
	for (int i = 0; i < Map->GetNumTilesets(); i++)
	{
		const Tmx::Tileset* set = Map->GetTileset(i);

		// Check if tileset texture is already loaded.
		if (Tileset_textures.find(set) != Tileset_textures.end())
			continue;

		int new_width, new_height, n_channels;
		unsigned char* data = getTextureData(this->directory + "/" + set->GetImage()->GetSource(), set->GetTileWidth(), set->GetTileHeight(), set->GetMargin(), set->GetSpacing(),
			new_width, new_height, n_channels);

		Texture2D t;
		t.Filter_mag = t.Filter_min = GL_NEAREST;
		t.Wrap_S = t.Wrap_T = GL_CLAMP_TO_EDGE;
		if (set->GetImage()->GetTransparentColor().IsTransparent()) {
			t.Internal_format = t.Image_format = GL_RGBA;
		}
		t.Generate((unsigned int)new_width, (unsigned int)new_height, data);

		Tmx::Tileset* s = Map->GetTileset_nonconst(i);
		s->SetMargin(1);
		s->SetSpacing(2);
		s->SetImagewidth(new_width);
		s->SetImageHeight(new_height);

		Tileset_textures[set] = t;

		delete[] data;
	}
}
void Tilemap::initAnimatedTiles()
{
	for (int i = 0; i < Map->GetNumTilesets(); i++)
	{
		const Tmx::Tileset* set = Map->GetTileset(i);
		for (auto& tile : set->GetTiles())
		{
			if (tile->IsAnimated())
			{
				auto at = new AnimatedTile(tile);				
				int gid = set->GetFirstGid() + tile->GetId();
				AnimTiles[gid] = at;
			}
		}
	}
}
void Tilemap::Update(float dt)
{
	for (auto i = AnimTiles.begin(); i != AnimTiles.end(); i++)
	{
		i->second->Update(dt);
	}
}
int Tilemap::GetTileId(const Tmx::TileLayer* layer, int x, int y)
{
	int gid = layer->GetTileGid(x, y);

	// If tile is animated. Then return tile ID of the current frame.
	if (AnimTiles.find(gid) != AnimTiles.end())
		return AnimTiles[gid]->GetCurrentTileID();

	return layer->GetTileId(x, y);
}

/* Animated Tile */
AnimatedTile::AnimatedTile(const Tmx::Tile* tile)
	: CurrentFrame(0), Frames(tile->GetFrames()), duration_left(0)
{
	CurrentFrame = 0;
	duration_left = (*Frames.begin()).GetDuration() / 1000.0f;
}
void AnimatedTile::Update(float dt)
{
	duration_left -= dt;
	if (duration_left <= 0)
	{
		CurrentFrame = (CurrentFrame + 1) % Frames.size();
		duration_left = Frames[CurrentFrame].GetDuration() / 1000.0f;
	}
}
int AnimatedTile::GetCurrentTileID()
{
	return Frames[CurrentFrame].GetTileID();
}
AnimatedTile::~AnimatedTile() {}