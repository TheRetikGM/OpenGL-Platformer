#pragma once
#include <unordered_map>
#include <string>
#include "shader.h"
#include "texture.h"
#include "tilemap.h"
#include "game/AnimationManager.h"

class ResourceManager
{
public:
	static std::unordered_map<std::string, std::unordered_map<std::string, Shader>>	Shaders;
	static std::unordered_map<std::string, std::unordered_map<std::string, Texture2D>> Textures;
	static std::unordered_map<std::string, std::unordered_map<std::string, Tilemap*>> Tilemaps;
	static std::unordered_map<std::string, std::unordered_map<std::string, AnimationManager*>> AnimationManagers;

	static Shader& 				LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name, std::string group = "");
	static Texture2D& 			LoadTexture(const char* file, bool alpha, std::string name, std::string group = "");
	static Tilemap* 			LoadTilemap(const char* file, std::string name, std::string group = "");	
	static AnimationManager* 	LoadAnimationManager(const char* file, std::string group = "");

	static Shader& 				GetShader(std::string name, std::string group = "");
	static Texture2D& 			GetTexture(std::string name, std::string group = "");
	static Tilemap* 			GetTilemap(std::string name, std::string group = "");
	static AnimationManager* 	GetAnimationManager(std::string name, std::string group = "");

	static void DeleteTexture(std::string name, std::string group = "");
	static void DeleteShader(std::string name, std::string group = "");
	static void DeleteTilemap(std::string name, std::string group = "");
	static void DeleteAnimationManager(std::string name, std::string group = "");

	static void DeleteGroup(std::string group);
	static void DeleteShaderGroup(std::string group, bool _erase_base = false);
	static void DeleteTextureGroup(std::string group, bool _erase_base = false);
	static void DeleteTilemapGroup(std::string group, bool _erase_base = false);
	static void DeleteAnimationManagerGroup(std::string group, bool _erase_base = false);

	static void Clear();
private:
	ResourceManager() { }
	static Shader loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = nullptr);
	static Texture2D loadTextureFromFile(const char* file, bool alpha);	
};