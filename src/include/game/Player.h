#pragma once
#include "game/GameObject.h"
#include "sprite_renderer.h"
#include "game/Sprite.h"
#include "Interfaces/ITileSpace.h"

enum class PlayerMovement : int {
	up, down, left, right
};

class Player : public GameObject, public ITileSpace
{
public:	
	float 		MovementSpeed;
	Sprite* 	PlayerSprite; 

	Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color);
	~Player();

	void Draw(SpriteRenderer* renderer);
	void DrawAt(SpriteRenderer* renderer, glm::vec2 pos);
	void Update(float dt);
	void ProcessKeyboard(PlayerMovement dir, float dt);

	void onTileSizeChanged(glm::vec2 newTileSize) override;
};