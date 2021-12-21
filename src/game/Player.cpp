#include "game/Player.h"
#include "camera/tileCamera2D.h"
#include "game/game.h"

#define ifFirstPressed(key, pred) if (keys[key] && !keys_processed[key]) { pred; keys_processed[key] = true; }

bool SlidingJumped = true;

Player::Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color)
	: GameObject(position, glm::vec2(0.0f, 0.0f), size, color)
	, PlayerSprite(sprite)
	, InCollision(false)
	, RBody(nullptr)
	, Animator(nullptr)
	, IsJumping(true)
	, Controls()
{	
	Game::AddTileSpaceObject(this);
	onTileSizeChanged(Game::TileSize);
}
Player::~Player()
{
	Game::RemoveTileSpaceObject(this);
}

void Player::SetRigidBody(std::shared_ptr<Physics2D::RigidBody> body)
{
	if (this->RBody) {
		body->OnCollisionEnter = [](Physics2D::RigidBody* b, const Physics2D::CollisionInfo& info) {};
		leftBody->OnCollisionEnter = [](Physics2D::RigidBody* b, const Physics2D::CollisionInfo& info) {};
		rightBody->OnCollisionEnter = [](Physics2D::RigidBody* b, const Physics2D::CollisionInfo& info) {};
	}
	this->RBody = body;
	body->OnCollisionEnter = std::bind(&Player::onCollision, this, std::placeholders::_1, std::placeholders::_2);;

	// TODO: initialize left and right bodies.
	const Physics2D::AABB& aabb = RBody->GetAABB();
	glm::vec2 sideBodySize = glm::vec2(sideBodyWidth, aabb.size.y);

	leftBody = Physics2D::RigidBody::CreateRectangleBody(aabb.position - glm::vec2(sideBodyWidth, 0.0f), sideBodySize, 1.0f, false, 0.0f);
	leftBody->Name = "leftBody";

	rightBody = Physics2D::RigidBody::CreateRectangleBody(aabb.position + glm::vec2(aabb.size.x, 0.0f), sideBodySize, 1.0f, false, 0.0f);
	rightBody->Name = "rightBody";

	// Side colliders should collider only with bodies other than this player body.
	leftBody->OnCollisionEnter = std::bind(&Player::onLeftCollision, this, std::placeholders::_1, std::placeholders::_2);
	rightBody->OnCollisionEnter = std::bind(&Player::onRightCollision, this, std::placeholders::_1, std::placeholders::_2);
	leftBody->IsKinematic = rightBody->IsKinematic = true;
}
void Player::AddToWorld(Physics2D::PhysicsWorld* world)
{
	if (this->RBody) {
		world->AddBody(RBody);
		world->AddBody(leftBody);
		world->AddBody(rightBody);
	}
}

void Player::Draw(SpriteRenderer* renderer)
{	
	PlayerSprite->Size = this->Size * Game::TileSize * TileCamera2D::GetScale();
	PlayerSprite->Position = GetScreenPosition();
	PlayerSprite->Draw(renderer);
}
void Player::DrawAt(SpriteRenderer* renderer, glm::vec2 pos)
{		
	renderer->DrawSprite(PlayerSprite->Texture, pos, PlayerSprite->Size, PlayerSprite->Rotation, PlayerSprite->Color);
}

bool in_range(float& num, float low, float high)
{
	return low <= num && num <= high;
}
template<typename T> T sign(T val) {
	return T((T(0) < val) - (val < T(0)));
}

void Player::Update(float dt)
{
	// Do collision test again in sorted order,
	// so that the player will not get stuck 
	// between tiles.
	std::sort(collisions.begin(), collisions.end(), [](const collision& a, const collision& b) { return a.dist < b.dist; });
	for (auto& c : collisions) {
		Physics2D::CollisionInfo info;
		if (Physics2D::CheckCollision(RBody.get(), c.body, info)) {
			if (c.body->Name == "ground")
			{
				if (glm::dot(info.normal, glm::vec2(0.0f, -1.0f)) > 0.0f)
				{
					CanJump = true;
					Velocity.y = 0.0f;
				}
			}
			RBody->MoveOutOfCollision(info);
		}
	}
	collisions.clear();
	UpdatePositions();
	// Move the left and right colliders.
	glm::vec2 offset = glm::vec2((RBody->GetAABB().size.x + sideBodyWidth) / 2.0f, 0.0f);
	leftBody->MoveTo(RBody->GetPosition() - offset);
	rightBody->MoveTo(RBody->GetPosition() + offset);
	leftBody->UpdatePosition();
	rightBody->UpdatePosition();

	if (Velocity.y > 0.0f && !SlidingWall)
		CanJump = false;

	if (Animator)
	{
		if (Velocity.x == 0.0f)
			Animator->SetParameter("state", "idle");
		else
			Animator->SetParameter("state", "run");

		if (IsJumping || !CanJump) {
			Animator->SetParameter("vertical", (Velocity.y < 0) ? 1 : -1);
		}
		else {
			Animator->SetParameter("vertical", 0);
		}
	}


	if (!leftColliding && !rightColliding)
		SlidingJumped = false;

	// Update the sprite.
	PlayerSprite = Animator->GetSprite();

	lastLeftColliding = leftColliding;
	lastRightColliding = rightColliding;
	leftColliding = rightColliding = false;
}

void Player::ProcessKeyboard(bool* keys, bool* keys_processed, float dt)
{
	int horizontal = 0;
	int vertical = 0;
	glm::vec2 rightVec = glm::vec2(1.0f, 0.0f);
	glm::vec2 leftVec = -rightVec;
	glm::vec2 upVec = glm::vec2(0.0f, -1.0f);
	glm::vec2 downVec = glm::vec2(0.0f, 1.0f);
	Acceleration = glm::vec2(0.0f, 0.0f);

	float hAcceleration = 20.0f;
	float hDeceleration = 30.0f;
	float maxHVelocity = 5.0f;
	bool hDecelerating = false;

	// Jump variables
	static float buttonTime = 0.5f;
	static float jumpHeight = 3.1f;
	static float cancelRate = 80.0f;
	static float jumpTime = 0.0f;
	static bool  jumpCanceled = false;
	static float JumpGravityScale = 2.0f;
	static float FallGravityScale = 5.0f;

	// Run left.
	if (keys[Controls.RunLeft])
	{
		if (Velocity.x > 0.0f)
			Acceleration.x += -hDeceleration;

		Acceleration += hAcceleration * leftVec;
		hDecelerating = false;
		horizontal += -1;
	}
	// Run right.
	if (keys[Controls.RunRight])
	{
		if (Velocity.x < 0.0f)
			Acceleration.x += hDeceleration;

		Acceleration += hAcceleration * rightVec;
		hDecelerating = false;
		horizontal += 1;
	}
	// Horizontal deceleration when not pressing left or right.
	if (Acceleration.x == 0.0f && Velocity.x != 0.0f)
	{
		Acceleration.x = -sign(Velocity.x) * hDeceleration;
		hDecelerating = true;
	}

	// Jump -- pressed once.
	if (keys[Controls.JumpUp] && !keys_processed[Controls.JumpUp])
	{
		if (CanJump)
		{ 
			float jump_impulse = std::sqrt(2.0f * Gravity * GravityScale * jumpHeight);
			Velocity.y = -jump_impulse;

			if (lastLeftColliding || lastRightColliding)
			{
				if (!SlidingJumped)
				{
					std::cout << "Slide jumped" << std::endl;
					Velocity.x = 1000.0f * (lastLeftColliding ? -1.0f : 1.0f);
					SlidingJumped = true;
				}
			}

			// Set default states.
			IsJumping = true;
			CanJump = false;
			jumpCanceled = false;
			jumpTime = 0.0f;
		}

		keys_processed[Controls.JumpUp] = true;
	}
	if (Velocity.y > 0.0)		// Falling
		GravityScale = FallGravityScale;
	else						// Jumping
		GravityScale = JumpGravityScale;
	if (IsJumping)
	{
		jumpTime += dt;
		// Check for canceled jump.
		if (!keys[Controls.JumpUp])
			jumpCanceled = true;
		// 
		if (jumpTime > buttonTime)
			IsJumping = false;
	}
	if (jumpCanceled && IsJumping && Velocity.y < 0.0f)
		Velocity += downVec * cancelRate * dt;

	Velocity += Acceleration * dt;
	// If hDecelerating and the body should stop, then stop it.
	if (Acceleration.x * Velocity.x > 0.0f && hDecelerating) {
		Velocity.x = 0.0f;
		hDecelerating = false;
	}

	// Clamp horizontal velocity to maximum speed.
	Velocity.x = glm::clamp(Velocity.x, -maxHVelocity, maxHVelocity);

	// Apply gravity and clamp it between ( -inf; gravity >
	Velocity.y += (Gravity * GravityScale) * dt;
	Velocity.y = glm::clamp(Velocity.y, -INFINITY, Gravity);

	// Move the player.
	RBody->Move(Velocity * dt);

	RBody->LinearVelocity = Velocity;
	RBody->Acceleration = Acceleration;

	if (horizontal != 0)
		Animator->SetParameter("horizontal", horizontal);
}
void Player::UpdatePositions()
{
	if (RBody) {
		this->Position = RBody->GetPosition();
	}
}
glm::vec2 Player::GetScreenPosition() 
{
	return TileCamera2D::GetScreenPosition(RBody->GetAABB().GetMin());
}

void Player::onCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info)
{
	if (body->Name == "leftBody" || body->Name == "rightBody")
		return;
	// Move the body out of the collision.
	// RBody->MoveOutOfCollision(info);
	glm::vec2 ab = body->GetPosition() - RBody->GetPosition();
	collisions.push_back(collision{ ab.x * ab.x + ab.y * ab.y, body });
}
void Player::onLeftCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info)
{
	if (body->Name == "ground" && glm::dot(glm::vec2(1.0f, 0.0f), info.normal) > 0.0f)
		this->leftColliding = !(body == this->RBody.get());
}
void Player::onRightCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info)
{
	if (body->Name == "ground" && glm::dot(glm::vec2(-1.0f, 0.0f), info.normal) > 0.0f)
		this->rightColliding = !(body == this->RBody.get());
}

void Player::SetPosition(glm::vec2 position)
{
	this->Position = position;
}

void Player::onTileSizeChanged(glm::vec2 newTileSize)
{
	// screenPosition = TileCamera2D::GetScreenPosition(Position) - (Size * Game::TileSize * TileCamera2D::GetScale()) / 2.0f;
}