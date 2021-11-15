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
	PlayerSprite->Position = this->ScreenPosition;
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
	if (RBody->LinearVelocity.y > 9.81f * 3.0f * dt)
		Jumping = true;

	if (Animator)
	{
		if (RBody->LinearVelocity.x == 0.0f)
			Animator->SetParameter("state", "idle");
		else
			Animator->SetParameter("state", "run");

		if (Jumping) {
			Animator->SetParameter("vertical", (RBody->LinearVelocity.y < 0) ? 1 : -1);
		}
		else {
			Animator->SetParameter("vertical", 0);
		}
	}
}
void Player::UpdatePositions()
{
	if (RBody) {
		this->Position = RBody->GetPosition();
		this->ScreenPosition = TileCamera2D::GetScreenPosition(RBody->GetAABB().GetMin());
	}
}

void Player::ProcessKeyboard(bool* keys, bool* keys_processed, float dt)
{
	int horizontal = 0;
	int vertical = 0;

	if (keys[Controls.JumpUp] && !keys_processed[Controls.JumpUp])
	{
		TileCamera2D::ProccessKeyboard(Camera2DMovement::up, dt);
		// player->ProcessKeyboard(PlayerMovement::up, dt);
		if (!Jumping) {
			Jumping = true;
			RBody->LinearVelocity.y = 0.0f;
			RBody->ApplyImpulse(glm::vec2(0.0f, -1.0f) * 25.0f);
			vertical = 1;
		}

		keys_processed[GLFW_KEY_W] = true;
	}
	if (!keys[Controls.JumpUp] && Jumping && RBody->LinearVelocity.y < 0.0f) {
		RBody->LinearVelocity.y = 100.0f * dt;
		vertical = 0;
	}
	if (keys[Controls.JumpDown])
	{
		// TileCamera2D::ProccessKeyboard(Camera2DMovement::down, dt);
	}
	float max_velocity = 10.0f;
	if (!in_range(RBody->LinearVelocity.x, -max_velocity, max_velocity)) {
		RBody->ClearForces();
		RBody->LinearVelocity.x = sign(RBody->LinearVelocity.x) * max_velocity;
	}
	else {
		if (keys[Controls.RunLeft])
		{
			TileCamera2D::ProccessKeyboard(Camera2DMovement::left, dt);
			// a = F / m
			// v = v0 + a * dt
			// 0 = v0 + (F / m) * dt
			// (-v0 / dt) * m = F
			glm::vec2 additional_force(0.0f);
			if (RBody->LinearVelocity.x > 0.0f)	// If moving right, add force based on the velocity.
				additional_force = glm::vec2(((-RBody->LinearVelocity.x) / (dt)) * RBody->Properties.Mass, 0.0f);
			RBody->AddForce(glm::vec2(-1.0f, 0.0f) * 500.0f + additional_force);
			horizontal = -1;
		}
		if (keys[Controls.RunRight])
		{
			TileCamera2D::ProccessKeyboard(Camera2DMovement::right, dt);

			glm::vec2 additional_force(0.0f);
			if (RBody->LinearVelocity.x < 0.0f)
				additional_force = glm::vec2(((-RBody->LinearVelocity.x) / (dt)) * RBody->Properties.Mass, 0.0f);

			RBody->AddForce(glm::vec2(1.0f, 0.0f) * 500.0f + additional_force);
			RBody->Properties.FrictionCoeff = 0.0f;
			horizontal = 1;
		}
	}
	if (!keys[Controls.RunLeft] && !keys[Controls.RunRight]) {
		//RBody->Properties.FrictionCoeff = 5.0f;
		// RBody->ApplyImpulse(glm::vec2(-RBody->LinearVelocity.x, 0.0f));
		RBody->LinearVelocity.x = 0.0f;
	}
	else {
		RBody->Properties.FrictionCoeff = 0.0f;
	}

	if (horizontal != 0)
		Animator->SetParameter("horizontal", horizontal);
}

void Player::onCollision(Physics2D::RigidBody* body, const Physics2D::CollisionInfo& info)
{
	if (body->Name == "ground" && glm::dot(info.normal, glm::vec2(0.0f, -1.0f)) > 0.0f)
		this->Jumping = false;
}

void Player::SetPosition(glm::vec2 position)
{
	this->Position = position;
	UpdateScreenPosition();
}
void Player::UpdateScreenPosition()
{
	ScreenPosition = TileCamera2D::GetScreenPosition(Position) - (Size * Game::TileSize * TileCamera2D::GetScale()) / 2.0f;
}

void Player::onTileSizeChanged(glm::vec2 newTileSize)
{
	ScreenPosition = TileCamera2D::GetScreenPosition(Position) - (Size * Game::TileSize * TileCamera2D::GetScale()) / 2.0f;
}