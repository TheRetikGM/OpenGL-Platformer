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

/*
* Handles menu input.
*  - Menu needs to be opened by referenced MenuManager.
*/
class MenuInputHandler
{
public:
    // NOTE: since C++17, static member variables marked as inline can be initialized inside a class.
    inline static MenuUpCommand        UpCommand;
    inline static MenuDownCommand      DownCommand;
    inline static MenuLeftCommand      LeftCommand;
    inline static MenuRightCommand     RightCommand;
    inline static MenuConfirmCommand   ConfirmCommand;
    inline static MenuBackCommand      BackCommand;
    MenuManager* pMenuManager;

    MenuInputHandler(MenuManager* manager) : pMenuManager(manager), controls()
    {
        // TODO: Maybe load these from some global config or file...
        controls[GLFW_KEY_W] = controls[GLFW_KEY_UP] = &MenuInputHandler::UpCommand;
        controls[GLFW_KEY_S] = controls[GLFW_KEY_DOWN] = &MenuInputHandler::DownCommand;
        controls[GLFW_KEY_A] = controls[GLFW_KEY_LEFT] = &MenuInputHandler::LeftCommand;
        controls[GLFW_KEY_D] = controls[GLFW_KEY_RIGHT] = &MenuInputHandler::RightCommand;
        controls[GLFW_KEY_SPACE] = controls[GLFW_KEY_ENTER] = &MenuInputHandler::ConfirmCommand;
        controls[GLFW_KEY_ESCAPE] = controls[GLFW_KEY_BACKSPACE] = &MenuInputHandler::BackCommand;
        controls[GLFW_KEY_G] = &MenuInputHandler::DownCommand;
    }
    ~MenuInputHandler() {}

    // Returns selected MenuObject or nullptr for other actions.
    MenuObject* HandleInput(InputInterface* input)
    {
        // Dont handle input for closed menu.
        if (pMenuManager->MenuClosed())
            return nullptr;

        // Check every control for keypress and execute given command. Exit after first match.
        for (auto& [key, command] : controls)
            if (input->Pressed(key))
                return command->execute(pMenuManager);
        return nullptr;
    }

protected:
    std::unordered_map<int, MenuCommand*> controls;
};