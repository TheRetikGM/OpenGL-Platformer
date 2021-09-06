#include "game/Player.h"
#include "camera/tileCamera2D.h"
#include "game/game.h"

Player::Player(glm::vec2 position, glm::vec2 size, Sprite* sprite, glm::vec3 color)
	: GameObject(position, glm::vec2(0.0f, 0.0f), size, color)
	, PlayerSprite(sprite)
	, MovementSpeed(2.0f)	
{	
	Game::AddTileSpaceObject(this);
	onTileSizeChanged(Game::TileSize);
}
Player::~Player()
{
	Game::RemoveTileSpaceObject(this);
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
}
void Player::ProcessKeyboard(PlayerMovement dir, float dt)
{
	glm::vec2 vel(0.0f);
	switch (dir)
	{
	case PlayerMovement::up:
		vel.y -= MovementSpeed * dt;
		break;
	case PlayerMovement::down:
		vel.y += MovementSpeed * dt;
		break;
	case PlayerMovement::left:
		vel.x -= MovementSpeed * dt;
		break;
	case PlayerMovement::right:
		vel.x += MovementSpeed * dt;
		break;
	}
	Position += vel;
	// Get screen position from tile-space position.
	glm::vec2 spos = TileCamera2D::GetScreenPosition(Position);
	glm::vec2 minus = (Size * Game::TileSize  * TileCamera2D::GetScale()) / 2.0f;
	//ScreenPosition = TileCamera2D::GetScreenPosition(Position) - (Size * Game::TileSize  * TileCamera2D::GetScale()) / 2.0f;
	ScreenPosition = spos - minus;	
}
void Player::onTileSizeChanged(glm::vec2 newTileSize)
{
	ScreenPosition = TileCamera2D::GetScreenPosition(Position) - (Size * Game::TileSize * TileCamera2D::GetScale()) / 2.0f;
}