#pragma once
#include "game/GameObject.h"
#include "sprite_renderer.h"
#include "game/Sprite.h"
#include "Interfaces/ITileSpace.h"
#include <memory>
#include <RigidBody.h>
#include <PhysicsWorld.h>
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
	Sprite* 	PlayerSprite;
	bool		InCollision;
	bool		CanJump;
	bool		IsJumping;
	bool		SlidingWall = false;
	std::shared_ptr<Physics2D::RigidBody> RBody;
	AnimationManager* Animator;
	PlayerControls Controls;
	const float Gravity = 9.81f;
	float GravityScale = 2.0f;
	
	Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color);
	~Player();

	void Draw(SpriteRenderer* renderer);
	void DrawAt(SpriteRenderer* renderer, glm::vec2 pos);
	void Update(float dt);
	void UpdatePositions();
	void ProcessKeyboard(bool* keys, bool* keys_processed, float dt);
	void SetRigidBody(std::shared_ptr<Physics2D::RigidBody> body);
	void SetPosition(glm::vec2 position);
	void AddToWorld(Physics2D::PhysicsWorld* world);

	glm::vec2 GetScreenPosition() override;
	void onTileSizeChanged(glm::vec2 newTileSize) override;
private:
	glm::vec2 lastSlidingDir = glm::vec2(0.0f, 0.0f);
	std::shared_ptr<Physics2D::RigidBody> leftBody;
	std::shared_ptr<Physics2D::RigidBody> rightBody;
	bool leftColliding = false;
	bool rightColliding = false;
	float sideBodyWidth = 0.3f;

	struct collision {
		float dist = INFINITY;
		Physics2D::RigidBody* body = nullptr;
	};
	std::vector<collision> collisions;

	void onCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info);
};