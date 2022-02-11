#pragma once
#include "game/Forms.hpp"
#include <functional>

typedef std::function<void()> SelectCallback;

class Dialog
{
public:
    Dialog(std::shared_ptr<Forms::Form> dialog_form, std::vector<std::pair<int, SelectCallback>> options)
        : pDialogForm(dialog_form), options(options)
    {}
    Dialog() = default;

    void ProcessInput(InputInterface* input)
    {
        for (auto& option : options)
        {
            if (input->Pressed(option.first))
            {
                option.second();
                return;
            }
        }
    }

    void Render(SpriteRenderer* sprite_renderer)
    {
        pDialogForm->Render(sprite_renderer);
    }

    Forms::Form* operator->() { return pDialogForm.operator->(); }
    std::shared_ptr<Forms::Form>& GetForm() { return pDialogForm; }

protected:
    std::shared_ptr<Forms::Form> pDialogForm = nullptr;

    // === Definition of dialog options ===
    // Define what should happen on option select.
    std::vector<std::pair<int, SelectCallback>> options;
};