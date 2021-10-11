#pragma once
#include "game/GameObject.h"
#include "sprite_renderer.h"
#include "game/Sprite.h"
#include "Interfaces/ITileSpace.h"
#include <memory>
#include "RigidBody.h"
#include "game/AnimationManager.h"

enum class PlayerMovement : int {
	up, down, left, right
};

class Player : public GameObject, public ITileSpace
{
public:	
	float 		MovementSpeed;
	Sprite* 	PlayerSprite;
	bool		InCollision;
	bool		Jumping;
	std::shared_ptr<Physics2D::RigidBody> RBody;
	AnimationManager* Animations;
	
	Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color);
	~Player();

	void Draw(SpriteRenderer* renderer);
	void DrawAt(SpriteRenderer* renderer, glm::vec2 pos);
	void Update(float dt);
	void ProcessKeyboard(PlayerMovement dir, float dt);
	void SetRigidBody(std::shared_ptr<Physics2D::RigidBody> body);

	void onTileSizeChanged(glm::vec2 newTileSize) override;
private:

	void onCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info);
};