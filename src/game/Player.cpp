#include "game/Player.h"
#include "camera/tileCamera2D.h"
#include "game/game.h"

#define ifFirstPressed(key, pred) if (keys[key] && !keys_processed[key]) { pred; keys_processed[key] = true; }

Player::Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color)
	: GameObject(position, glm::vec2(0.0f, 0.0f), size, color)
	, PlayerSprite(sprite)
	, MovementSpeed(2.0f)
	, InCollision(false)
	, RBody(nullptr)
	, Animator(nullptr)
	, Jumping(true)
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
	if (this->RBody)
		body->OnCollisionEnter = [](Physics2D::RigidBody* b, const Physics2D::CollisionInfo& info) {};
	this->RBody = body;
	body->OnCollisionEnter = std::bind(&Player::onCollision, this, std::placeholders::_1, std::placeholders::_2);
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

void Player::ProcessKeyboard(bool* keys, bool* keys_processed, float dt)
{
	int horizontal = 0;
	int vertical = 0;
	glm::vec2 rightVec = glm::vec2(1.0f, 0.0f);
	glm::vec2 leftVec = -rightVec;
	glm::vec2 upVec = glm::vec2(0.0f, -1.0f);
	glm::vec2 downVec = glm::vec2(0.0f, 1.0f);

	float hAcceleration = 15.0f;		// After 1 second the velocity will be 3 tiles per s.
	float hDeceleration = 30.0f;
	float maxHVelocity = 5.0f;
	bool hDecelerating = false;
	float vAcceleration = 30.0f;
	float maxJumpHeight = 3.0f * this->Size.y;
	Acceleration = glm::vec2(0.0f, 0.0f);

	if (keys[Controls.RunLeft])
	{
		Acceleration += hAcceleration * leftVec;
		hDecelerating = false;
		horizontal += -1;
	}
	if (keys[Controls.RunRight])
	{
		Acceleration += hAcceleration * rightVec;
		hDecelerating = false;
		horizontal += 1;
	}
	if (Acceleration.x == 0.0f && Velocity.x != 0.0f)
	{
		Acceleration.x = -sign(Velocity.x) * hDeceleration;
		hDecelerating = true;
	}

	if (keys[Controls.JumpUp] && !keys_processed[Controls.JumpUp])
	{
		if (!Jumping)
		{
			Velocity.y = -10.0;
			Jumping = true;
		}

		keys_processed[Controls.JumpUp] = true;
	}

	Velocity += Acceleration * dt;
	// If hDecelerating and the body should stop, then stop it.
	if (Acceleration.x * Velocity.x > 0.0f && hDecelerating) {
		Velocity.x = 0.0f;
		hDecelerating = false;
	}

	// Clamp horizontal velocity to maximum speed.
	Velocity.x = glm::clamp(Velocity.x, -maxHVelocity, maxHVelocity);

	Velocity.y += Gravity * dt;
	Velocity.y = glm::clamp(Velocity.y, -INFINITY, Gravity);

	// Move the player.
	RBody->Move(Velocity * dt);

	RBody->LinearVelocity = Velocity;
	RBody->Acceleration = Acceleration;

	if (horizontal != 0)
		Animator->SetParameter("horizontal", horizontal);
}

void Player::Update(float dt)
{
	std::sort(collisions.begin(), collisions.end(), [](const collision& a, const collision& b) { return a.dist < b.dist; });
	for (auto& c : collisions) {
		Physics2D::CollisionInfo info;
		if (Physics2D::CheckCollision(RBody.get(), c.body, info)) {
			if (c.body->Name == "ground" && glm::dot(info.normal, glm::vec2(0.0f, -1.0f)) > 0.0f)
			{
				Jumping = false;
				Velocity.y = 0.0f;
			}
			RBody->MoveOutOfCollision(info);
		}
	}
	collisions.clear();
	UpdatePositions();

	if (Velocity.y == Gravity)
		Jumping = true;

	if (Animator)
	{
		if (Velocity.x == 0.0f)
			Animator->SetParameter("state", "idle");
		else
			Animator->SetParameter("state", "run");

		if (Jumping) {
			Animator->SetParameter("vertical", (Velocity.y < 0) ? 1 : -1);
		}
		else {
			Animator->SetParameter("vertical", 0);
		}
	}

	// Update the sprite.
	PlayerSprite = Animator->GetSprite();
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
	// Move the body out of the collision.
	// RBody->MoveOutOfCollision(info);
	glm::vec2 ab = body->GetPosition() - RBody->GetPosition();
	collisions.push_back(collision{ ab.x * ab.x + ab.y * ab.y, body });
	if (body->Name == "ground")
	{
	}
}

void Player::SetPosition(glm::vec2 position)
{
	this->Position = position;
}

void Player::onTileSizeChanged(glm::vec2 newTileSize)
{
	// screenPosition = TileCamera2D::GetScreenPosition(Position) - (Size * Game::TileSize * TileCamera2D::GetScale()) / 2.0f;
}