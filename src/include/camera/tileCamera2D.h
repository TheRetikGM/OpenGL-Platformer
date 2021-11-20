#pragma once
#include "game/GameObject.h"
#include <functional>
#include "Interfaces/ITileSpace.h"
#define DEFAULT_CAMERA_MOVE_SPEED 250.0f

enum class Camera2DMovement : int { up, down, left, right };

class TileCamera2D
{
public:
	static glm::vec2*  Position;		// Can be position of the followed object if following. Otherwise Camera uses its own position.
    static float       MoveSpeed;        
	static glm::vec2   ScreenCoords;
	static GameObject* Follow;
	static std::function<void (glm::vec2)> OnScale;
	/// Size of the tilemap curently rendered. Size is in Tile space.
	static glm::vec2 CurrentMapSize;

	static glm::mat4 GetViewMatrix();
	static void ProccessKeyboard(Camera2DMovement direction, float delta);
	static void Rotate(float angle);
	static glm::vec2 GetRight() { return right; };
	static void SetRight(glm::vec2 right);
	static void RenderAtPosition(BasicRenderer* renderer, br_Shape shape, glm::vec3 color = glm::vec3(1.0f));

	// Return first Top-Left visible tile x,y.
	static glm::vec2 GetFirstVisibleTile();
	static glm::ivec2 GetNVisibleTiles();
	static void Update(float dt);
	static glm::vec2 GetScreenPosition(glm::vec2 tileSpacePosition);
	static glm::vec2 GetTileSpacePosition(glm::vec2 screenSpacePosition);
	static void SetFollow(GameObject* follow_obj);
	static void UnsetFollow(bool keep_position = false);
	// Sets LOCAL position of camera. Does not affect position of followed object.
	static void SetPosition(glm::vec2 pos);	

	// Get;Set
	static void SetScale(glm::vec2 scale);
	static glm::vec2 GetScale();
private:
	static glm::vec2 right;
	static glm::vec2 scale;

	// Position of camera if it isn't following anything.
	static glm::vec2 position;
};