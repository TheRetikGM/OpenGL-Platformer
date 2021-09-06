#pragma once
#include <map>
#include <string>
#include "shader.h"
#include "texture.h"
#include "tilemap.h"
#include "game/AnimationManager.h"

class ResourceManager
{
public:
	static std::map<std::string, Shader>	Shaders;
	static std::map<std::string, Texture2D> Textures;
	static std::map<std::string, Tilemap*>	Tilemaps;
	static std::map<std::string, AnimationManager*> AnimationManagers;

	static Shader LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name);
	static Shader GetShader(std::string name);
	static Texture2D LoadTexture(const char* file, bool alpha, std::string name);
	static Texture2D GetTexture(std::string name);
	static Tilemap* LoadTilemap(const char* file, std::string name);	
	static Tilemap* GetTilemap(std::string name);
	static AnimationManager* LoadAnimationManager(const char* file);
	static AnimationManager* GetAnimationManager(std::string name);
	static void Clear();
private:
	ResourceManager() { }
	static Shader loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = nullptr);
	static Texture2D loadTextureFromFile(const char* file, bool alpha);	
};