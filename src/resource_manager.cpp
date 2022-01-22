#include "resource_manager.h"
#include <exception>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stb_image.h>
#include <stdexcept>
#include "config.h"
#include "DebugColors.h"
#include <any>

std::unordered_map<std::string, std::unordered_map<std::string, Shader>> ResourceManager::Shaders;
std::unordered_map<std::string, std::unordered_map<std::string, Texture2D>> ResourceManager::Textures;
std::unordered_map<std::string, std::unordered_map<std::string, Tilemap*>> ResourceManager::Tilemaps;
std::unordered_map<std::string, std::unordered_map<std::string, AnimationManager*>> ResourceManager::AnimationManagers;

Shader& ResourceManager::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name, std::string group)
{
	try
	{
		Shaders[group][name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
	}
	catch (const std::exception& e)
	{
		std::cerr << DC_ERROR " ResourceManager::LoadShader(): " << e.what() << '\n';
	}

	return Shaders[group][name];
}
Shader& ResourceManager::GetShader(std::string name, std::string group)
{
	return Shaders.at(group).at(name);
}
void ResourceManager::DeleteShader(std::string name, std::string group)
{
	glDeleteProgram(Shaders.at(group).at(name).ID);
	Shaders.at(group).erase(name);
}

Texture2D& ResourceManager::LoadTexture(const char* file, bool alpha, std::string name, std::string group)
{
	try
	{
		Textures[group][name] = loadTextureFromFile(file, alpha);
	}
	catch (const std::exception& e)
	{
		std::cerr << DC_ERROR " ResourceManager::LoadTexture(): " << e.what() << '\n';
	}
	return Textures[group][name];
}
Texture2D& ResourceManager::GetTexture(std::string name, std::string group)
{
	return Textures.at(group).at(name);
}
void ResourceManager::DeleteTexture(std::string name, std::string group)
{
	glDeleteTextures(1, &Textures.at(group).at(name).ID);
	Textures.at(group).erase(name);
}

Shader ResourceManager::loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	try
	{
		std::ifstream vertexShaderFile(vShaderFile);
		std::ifstream fragmentShaderFile(fShaderFile);
		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vertexShaderFile.rdbuf();
		fShaderStream << fragmentShaderFile.rdbuf();

		vertexShaderFile.close();		
		fragmentShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

		if (gShaderFile != nullptr)
		{
			std::ifstream geometryShaderFile(gShaderFile);
			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::exception& e)
	{
		throw std::runtime_error(("Failed to read shader files\n" + std::string(e.what())).c_str());
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();

	Shader shader;
	shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
	return shader;
}
Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha)
{
	Texture2D texture;
	if (alpha)
	{
		texture.Internal_format = GL_RGBA;
		texture.Image_format = GL_RGBA;
	}
	int width, height, nrChannels;
	unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);

	if (!data)
		throw std::runtime_error(("Could not load texture '" + std::string(file) + "'.").c_str());
	texture.Generate(width, height, data);
	stbi_image_free(data);
	return texture;
}

Tilemap* ResourceManager::LoadTilemap(const char* file, std::string name, std::string group)
{
	Tilemap* t = nullptr;
	try {
		t = new Tilemap(file);
		Tilemaps[group][name] = t;
	}
	catch (const std::exception& e)
	{
		std::cerr << DC_ERROR " ResourceManager::LoadTilemap(): " << e.what() << std::endl;
	}
	return t;
}
Tilemap* ResourceManager::GetTilemap(std::string name, std::string group)
{
	return Tilemaps.at(group).at(name);
}
void ResourceManager::DeleteTilemap(std::string name, std::string group)
{
	delete Tilemaps.at(group).at(name);
	Tilemaps.at(group).erase(name);
}

AnimationManager* ResourceManager::LoadAnimationManager(const char* file, std::string group)
{
	std::ifstream jsonFile(file);
	if (!jsonFile.is_open())
		throw std::runtime_error("Could not load AnimationManager at '" + std::string(file) + "'.");
	std::stringstream json_data;
	json_data << jsonFile.rdbuf();

	nlohmann::json j;
	json_data >> j;

	auto man = j.get<AnimationManager>();
	AnimationManagers[group][man.Name] = new AnimationManager();
	*AnimationManagers[group][man.Name] = man;	

	return AnimationManagers[group][man.Name];
}
AnimationManager* ResourceManager::GetAnimationManager(std::string name, std::string group)
{
	return AnimationManagers.at(group).at(name);
}
void ResourceManager::DeleteAnimationManager(std::string name, std::string group)
{
	AnimationManagers.at(group).at(name)->DeleteAllTextures();
	AnimationManager* ptr = AnimationManagers.at(group).at(name);
	delete ptr;
	AnimationManagers.at(group).erase(name);
}

void ResourceManager::DeleteGroup(std::string group)
{
	if (Shaders.find(group) != Shaders.end())
		DeleteShaderGroup(group);
	if (Textures.find(group) != Textures.end())
		DeleteTextureGroup(group);
	if (Tilemaps.find(group) != Tilemaps.end())
		DeleteTilemapGroup(group);
	if (AnimationManagers.find(group) != AnimationManagers.end())
		DeleteAnimationManagerGroup(group);
}
void ResourceManager::DeleteShaderGroup(std::string sGroupName, bool _erase_base)
{
	auto& group = Shaders.at(sGroupName);
	std::vector<std::string> names;
	for (auto& pair : group)
		names.push_back(pair.first);
	for (auto& name : names)
		DeleteShader(name, sGroupName);
	if (_erase_base)
		Shaders.erase(sGroupName);
}
void ResourceManager::DeleteTextureGroup(std::string sGroupName, bool _erase_base)
{
	auto& group = Textures.at(sGroupName);
	std::vector<std::string> names;
	for (auto& pair : group)
		names.push_back(pair.first);
	for (auto& name : names)
		DeleteTexture(name, sGroupName);
	if (_erase_base)
		Textures.erase(sGroupName);
}
void ResourceManager::DeleteTilemapGroup(std::string sGroupName, bool _erase_base)
{
	auto& group = Tilemaps.at(sGroupName);
	std::vector<std::string> names;
	for (auto& pair : group)
		names.push_back(pair.first);
	for (auto& name : names)
		DeleteTilemap(name, sGroupName);
	if (_erase_base)
		Tilemaps.erase(sGroupName);
}
void ResourceManager::DeleteAnimationManagerGroup(std::string sGroupName, bool _erase_base)
{
	auto& group = AnimationManagers.at(sGroupName);
	std::vector<std::string> names;
	for (auto& pair : group)
		names.push_back(pair.first);
	for (auto& name : names)
		DeleteAnimationManager(name, sGroupName);
	if (_erase_base)
		AnimationManagers.erase(sGroupName);
}

void ResourceManager::Clear()
{
	for (auto& i : Shaders)
		DeleteShaderGroup(i.first, false);
	for (auto& i : Textures)
		DeleteTextureGroup(i.first, false);
	for (auto& i : Tilemaps)
		DeleteTilemapGroup(i.first, false);
	for (auto& i : AnimationManagers)
		DeleteAnimationManagerGroup(i.first, false);

	Shaders.clear();
	Textures.clear();
	Tilemaps.clear();
	AnimationManagers.clear();
}