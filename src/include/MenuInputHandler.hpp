#pragma once
#include "MenuSystem.hpp"
#include "GLFW/glfw3.h"
#include "InputInterface.hpp"
#include <unordered_map>

using namespace MenuSystem;

class MenuCommand
{
public:
    virtual MenuObject* execute(MenuManager* pMan) = 0;
};
class MenuUpCommand : public MenuCommand
{
public:
    MenuObject* execute(MenuManager* pMan) { pMan->OnUp(); return nullptr; }
};
class MenuDownCommand : public MenuCommand
{
public:
    MenuObject* execute(MenuManager* pMan) { pMan->OnDown(); return nullptr; }
};
class MenuLeftCommand : public MenuCommand
{
public:
    MenuObject* execute(MenuManager* pMan) { pMan->OnLeft(); return nullptr; }
};
class MenuRightCommand : public MenuCommand
{
public:
    MenuObject* execute(MenuManager* pMan) { pMan->OnRight(); return nullptr; }
};
class MenuConfirmCommand : public MenuCommand
{
public:
    MenuObject* execute(MenuManager* pMan) { return pMan->OnConfirm(); }
};
class MenuBackCommand : public MenuCommand
{
public:
    MenuObject* execute(MenuManager* pMan) { pMan->OnBack(); return nullptr; }
};

class MenuInputHandler
{
public:
    inline static MenuUpCommand        UpCommand;
    inline static MenuDownCommand      DownCommand;
    inline static MenuLeftCommand      LeftCommand;
    inline static MenuRightCommand     RightCommand;
    inline static MenuConfirmCommand   ConfirmCommand;
    inline static MenuBackCommand      BackCommand;
    MenuManager* pMenuManager;

    MenuInputHandler(MenuManager* manager) : pMenuManager(manager), controls()
    {
        controls[GLFW_KEY_W] = controls[GLFW_KEY_UP] = &MenuInputHandler::UpCommand;
        controls[GLFW_KEY_S] = controls[GLFW_KEY_DOWN] = &MenuInputHandler::DownCommand;
        controls[GLFW_KEY_A] = controls[GLFW_KEY_LEFT] = &MenuInputHandler::LeftCommand;
        controls[GLFW_KEY_D] = controls[GLFW_KEY_RIGHT] = &MenuInputHandler::RightCommand;
        controls[GLFW_KEY_SPACE] = controls[GLFW_KEY_ENTER] = &MenuInputHandler::ConfirmCommand;
        controls[GLFW_KEY_ESCAPE] = controls[GLFW_KEY_BACKSPACE] = &MenuInputHandler::BackCommand;
    }
    ~MenuInputHandler() {}

    MenuObject* HandleInput(InputInterface* input)
    {
        if (pMenuManager->MenuClosed())
            return nullptr;

        for (auto& [key, command] : controls)
            if (input->Pressed(key))
                return command->execute(pMenuManager);
        return nullptr;
    }

protected:
    std::unordered_map<int, MenuCommand*> controls;
};