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

std::map<std::string, Shader> ResourceManager::Shaders;
std::map<std::string, Texture2D> ResourceManager::Textures;
std::map<std::string, Tilemap*> ResourceManager::Tilemaps;
std::map<std::string, AnimationManager*> ResourceManager::AnimationManagers;
Shader ResourceManager::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name)
{
	try
	{
		Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
	}
	catch (const std::exception& e)
	{
		std::cerr << DC_ERROR " ResourceManager::LoadShader(): " << e.what() << '\n';
	}

	return Shaders[name];
}
Shader ResourceManager::GetShader(std::string name)
{
	return Shaders[name];
}
void ResourceManager::DeleteShader(std::string name)
{
	glDeleteProgram(Shaders.at(name).ID);
	Shaders.erase(name);
}
Texture2D& ResourceManager::LoadTexture(const char* file, bool alpha, std::string name)
{
	try
	{
		Textures[name] = loadTextureFromFile(file, alpha);
	}
	catch (const std::exception& e)
	{
		std::cerr << DC_ERROR " ResourceManager::LoadTexture(): " << e.what() << '\n';
	}
	return Textures[name];
}
Texture2D& ResourceManager::GetTexture(std::string name)
{
	return Textures[name];
}
void ResourceManager::DeleteTexture(std::string name)
{
	glDeleteTextures(1, &Textures.at(name).ID);
	Textures.erase(name);
}
void ResourceManager::Clear()
{
	for (auto& i : Shaders)
		glDeleteProgram(i.second.ID);
	for (auto& i : Textures)
		glDeleteTextures(1, &i.second.ID);
	for (auto& i : Tilemaps)
		delete i.second;
	for (auto& [name, manager] : AnimationManagers)
	{
		manager->DeleteAllTextures();
		delete manager;
	}
	Shaders.clear();
	Textures.clear();
	Tilemaps.clear();
	AnimationManagers.clear();
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
Tilemap* ResourceManager::LoadTilemap(const char* file, std::string name)
{
	Tilemap* t = nullptr;
	try {
		t = new Tilemap(file);
		Tilemaps[name] = t;
	}
	catch (const std::exception& e)
	{
		std::cerr << DC_ERROR " ResourceManager::LoadTilemap(): " << e.what() << std::endl;
	}
	return t;
}
Tilemap* ResourceManager::GetTilemap(std::string name)
{
	return Tilemaps[name];
}
void ResourceManager::DeleteTilemap(std::string name)
{
	delete Tilemaps.at(name);
	Tilemaps.erase(name);
}
AnimationManager* ResourceManager::LoadAnimationManager(const char* file)
{
	std::ifstream jsonFile(file);
	if (!jsonFile.is_open())
		throw std::runtime_error("Could not load AnimationManager at '" + std::string(file) + "'.");
	std::stringstream json_data;
	json_data << jsonFile.rdbuf();

	nlohmann::json j;
	json_data >> j;

	auto man = j.get<AnimationManager>();
	AnimationManagers[man.Name] = new AnimationManager();
	*AnimationManagers[man.Name] = man;	

	return AnimationManagers[man.Name];
}
AnimationManager* ResourceManager::GetAnimationManager(std::string name)
{
	return AnimationManagers[name];
}
void ResourceManager::DeleteAnimationManager(std::string name)
{
	AnimationManagers.at(name)->DeleteAllTextures();
	delete AnimationManagers.at(name);
	AnimationManagers.erase(name);
}