#pragma once
#include "game/GameObject.h"
#include "sprite_renderer.h"
#include "game/Sprite.h"
#include "game/TileSpace.h"
#include "BasicObserverSubject.hpp"
#include "game/AnimationManager.h"
#include "GLFW/glfw3.h"
#include "InputInterface.hpp"
#include "Timer.hpp"
#include <memory>
#include <RigidBody.h>
#include <PhysicsWorld.h>

class PlayerControls {
public:
	int JumpUp = GLFW_KEY_W;
	int JumpDown = GLFW_KEY_S;
	int RunLeft = GLFW_KEY_A;
	int RunRight = GLFW_KEY_D;
	int Attack = GLFW_KEY_LEFT_SHIFT;

	PlayerControls() {}
};

class Player : public GameObject, public TileSpace, public BasicObserverSubject
{
public:
	int			Lives;		// The amount of hearts.
	bool		InCollision;
	bool		CanJump;
	bool		IsJumping;
	bool		SlidingWall = false;
	std::shared_ptr<Physics2D::RigidBody> RBody;
	AnimationManager* Animator;
	PlayerControls Controls;
	const float Gravity = 9.81f;
	float GravityScale = 2.0f;
	bool canWallJump = false;
	glm::vec2 wallNormal;
	bool bCanLoseLife = true;
	
	Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color);
	virtual ~Player();

	void Draw(SpriteRenderer* renderer);
	void DrawAt(SpriteRenderer* renderer, glm::vec2 pos);
	void Update(float dt);
	void UpdatePositions();
	void ProcessKeyboard(InputInterface* input, float dt);

	void SetRigidBody(std::shared_ptr<Physics2D::RigidBody> body);
	void SetPosition(glm::vec2 position);
	void SetSpriteOffset(glm::vec2 offset_in_tiles);
	void SetSprite(Sprite* spr);
	Sprite* GetSprite() { return playerSprite; }
	void OnHit();

	void AddToWorld(Physics2D::PhysicsWorld* world);

	glm::vec2 GetScreenPosition() override;
	void onTileSizeChanged(glm::vec2 newTileSize) override;

protected:
	Sprite* 	playerSprite;
	float 		spriteRatio;
	glm::vec2	spriteOffset;

	// Invincibility.
	float fInvincibilityDuration = 1.0f;	// in seconds
	Timer invincibilityTimer;

	glm::vec2 lastSlidingDir = glm::vec2(0.0f, 0.0f);
	std::shared_ptr<Physics2D::RigidBody> leftBody;
	std::shared_ptr<Physics2D::RigidBody> rightBody;

	float sideBodyWidth = 0.1f;

	struct collision {
		float dist = INFINITY;
		Physics2D::RigidBody* body = nullptr;
	};
	std::vector<collision> collisions;

	void onCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info);
	void onLeftCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info);
	void onRightCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info);
};