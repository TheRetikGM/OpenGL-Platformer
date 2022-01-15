#include "game/GameStates.h"

void ActiveLevelState::Init()
{
    InLevelState::Init();
}
IGameState* ActiveLevelState::ProcessInput(bool* keys, bool* keys_processed, float dt)
{
    pPlayer->ProcessKeyboard(keys, keys_processed, dt);

    // ==== TODO: move this into player's code
    auto playerAnim = ResourceManager::GetAnimationManager("PlayerAnimations");
    if (keys[GLFW_KEY_LEFT_SHIFT] && !keys_processed[GLFW_KEY_LEFT_SHIFT])
    {
        playerAnim->PlayOnce("attack");
        keys_processed[GLFW_KEY_LEFT_SHIFT] = true;
    }

    if (keys[GLFW_KEY_ESCAPE] && !keys_processed[GLFW_KEY_ESCAPE])
    {
        // this->State = GameState::ingame_paused;	
        // menu_manager->Open(&menu->at("Main Menu"));

        // TODO: Create new state and return it;
        keys_processed[GLFW_KEY_ESCAPE] = true;
    }

    return nullptr;
}
IGameState* ActiveLevelState::Update(float dt)
{
    TileCamera2D::Update(dt);
    pLevel->Update(dt);
    pPlayer->Update(dt);
    pPlayer->SetSprite(pPlayer->Animator->GetSprite());

    for (auto& [name, manager] : ResourceManager::AnimationManagers)
			manager->Update(dt);
}