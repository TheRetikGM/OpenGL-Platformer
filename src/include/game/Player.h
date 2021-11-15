#pragma once
#include "game/GameObject.h"
#include "sprite_renderer.h"
#include "game/Sprite.h"
#include "Interfaces/ITileSpace.h"
#include <memory>
#include "RigidBody.h"
#include "game/AnimationManager.h"
#include "GLFW/glfw3.h"

class PlayerControls {
public:
	int JumpUp = GLFW_KEY_W;
	int JumpDown = GLFW_KEY_S;
	int RunLeft = GLFW_KEY_A;
	int RunRight = GLFW_KEY_D;

	PlayerControls() {}
};

class Player : public GameObject, public ITileSpace
{
public:	
	float 		MovementSpeed;
	Sprite* 	PlayerSprite;
	bool		InCollision;
	bool		Jumping;
	std::shared_ptr<Physics2D::RigidBody> RBody;
	AnimationManager* Animator;
	PlayerControls Controls;
	
	Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color);
	~Player();

	void Draw(SpriteRenderer* renderer);
	void DrawAt(SpriteRenderer* renderer, glm::vec2 pos);
	void Update(float dt);
	void UpdatePositions();
	void ProcessKeyboard(bool* keys, bool* keys_processed, float dt);
	void SetRigidBody(std::shared_ptr<Physics2D::RigidBody> body);
	void SetPosition(glm::vec2 position);
	void UpdateScreenPosition();

	void onTileSizeChanged(glm::vec2 newTileSize) override;
private:

	void onCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info);
};