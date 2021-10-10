#include "game/Player.h"
#include "camera/tileCamera2D.h"
#include "game/game.h"

Player::Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color)
	: GameObject(position, glm::vec2(0.0f, 0.0f), size, color)
	, PlayerSprite(sprite)
	, MovementSpeed(2.0f)
	, InCollision(false)
	, RBody(nullptr)
	, Animations(nullptr)
	, Jumping(true)
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
		body->OnCollisionEnter = [](Physics2D::RigidBody* b) {};
	this->RBody = body;
	body->OnCollisionEnter = std::bind(&Player::onCollision, this, std::placeholders::_1);
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
void Player::Update(float dt)
{
	if (Animations)
	{
		if (RBody->GetForce() == glm::vec2(0.0f, 0.0f) && !Jumping && RBody->LinearVelocity.x == 0.0f)
		{
			Animations->SetParameter("state", "idle");
			Animations->SetParameter("vertical", 0);
		}
		else if (!Jumping) {
			Animations->SetParameter("state", "run");
			Animations->SetParameter("vertical", 0);
		}
		else {
			glm::vec2 norm_vel = glm::normalize(RBody->LinearVelocity);
			float up_dot = glm::dot(norm_vel, glm::vec2(0.0f, -1.0f));

			int vertical = up_dot != 0.0f ? (up_dot > 0.0f ? 1 : -1) : 0;
			
			Animations->SetParameter("state", "run");			
			Animations->SetParameter("vertical", vertical);
		}

	}
	if (RBody) {
		this->Position = RBody->GetPosition();
		this->ScreenPosition = TileCamera2D::GetScreenPosition(RBody->GetPosition() - RBody->GetAABBSize() / 2.0f);
	}
}
void Player::ProcessKeyboard(PlayerMovement dir, float dt)
{
	glm::vec2 vel(0.0f);
	switch (dir)
	{
	case PlayerMovement::up:
		vel.y -= 1.0f;
		break;
	case PlayerMovement::down:
		vel.y += 0.0f;
		break;
	case PlayerMovement::left:
		vel.x -= 1.0f;
		break;
	case PlayerMovement::right:
		vel.x += 1.0f;
		break;
	}	
	if (RBody)
	{
		glm::vec2 force = glm::vec2(vel.x, 0.0f) * 1000.0f;
		if (vel.y == -1.0f && !Jumping)
		{
			Jumping = true;
			force += glm::vec2(0.0f, -1.0f) * 100000.0f;
		}
		RBody->AddForce(force);				

		// RBody->Move(glm::vec2(vel.x, 0.0f) * 3.0f * dt, true);
	}
	else {
		Position += vel;
		// Get screen position from tile-space position.
		ScreenPosition = TileCamera2D::GetScreenPosition(Position) - (Size * Game::TileSize * TileCamera2D::GetScale()) / 2.0f;
	}

	if ((int)vel.x != 0)
		Animations->SetParameter("horizontal", (int)vel.x);
}

void Player::onCollision(Physics2D::RigidBody* body)
{
	if (body->Name == "ground")
		this->Jumping = false;
}

void Player::onTileSizeChanged(glm::vec2 newTileSize)
{
	ScreenPosition = TileCamera2D::GetScreenPosition(Position) - (Size * Game::TileSize * TileCamera2D::GetScale()) / 2.0f;
}