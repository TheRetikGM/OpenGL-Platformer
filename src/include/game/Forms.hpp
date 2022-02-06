#pragma once
#include "InputInterface.hpp"
#include "game/game.h"
#include "AtlasTextRenderer.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include "BasicObserverSubject.hpp"
#define MSG_SIZE_CHANGED 0x1

namespace Forms
{
    class IRenderable
    {
    public:
        glm::vec2 vOffset = glm::vec2(0.0f);
        glm::vec2 vPosition = glm::vec2(0.0f);
        glm::vec2 vSize = glm::vec2(0.0f);

        IRenderable(glm::vec2 vPosition, glm::vec2 vSize) : vPosition(vPosition), vSize(vSize) {}
        IRenderable() = default;

        virtual void Render(SpriteRenderer* pSpriteRenderer) = 0;
    };

    enum class ControlType : int { label = 0, pair = 1 };
    class Control : public IRenderable, public BasicObserverSubject
    {
    public:
        // Name to unique indentification.
        std::string sName = "";
        glm::vec3 vColor = glm::vec3(1.0f);

        Control(glm::vec2 position, glm::vec2 size, glm::vec3 color) : IRenderable(position, size), vColor(color) {}
        Control() = default;

        // Implementation of IRenderable
        virtual glm::vec2 GetPosition() { return vPosition; }
        virtual glm::vec2 GetSize() { return vSize; }
        virtual void SetOffset(glm::vec2 offset) { this->vOffset = offset; }
        virtual void SetPosition(glm::vec2 pos) { this->vPosition = pos; }
    };

    class Label : public Control
    {
    public:
        AtlasTextRenderer* pTextRenderer;

        // Position in screen-space
        // font_size -- size of the monospace font.
        Label(std::string sText, glm::vec2 position, glm::vec2 font_size, glm::vec3 color, AtlasTextRenderer* text_renderer)
            : pTextRenderer(text_renderer)
        {
            Control::vPosition = position;
            Control::vColor = color;
            vTextScale = font_size / text_renderer->vCharSize;
            SetText(sText);   
        }
        void SetText(std::string text)
        {
            sText = text;
            Control::vSize = pTextRenderer->GetStringSize(sText, vTextScale);
            notify(MSG_SIZE_CHANGED);
        }

        virtual void Render(SpriteRenderer* pSpriteRenderer)
        {
            pTextRenderer->RenderText(pSpriteRenderer, sText, vOffset + vPosition, vTextScale, vColor);
        }
    protected:
        glm::vec2 vTextScale = glm::vec2(1.0f);
        std::string sText = "";
    };

    class Pair : public Control
    {
    public:
        std::shared_ptr<Control> pFirst = nullptr;
        std::shared_ptr<Control> pSecond = nullptr;
        int nSpacing = 7;

        Pair(std::shared_ptr<Control> first, std::shared_ptr<Control> second, glm::vec2 position)
            : pFirst(first), pSecond(second)
        {
            Control::vPosition = position;

            SetPosition(position);

            // Compute size.
            glm::vec2 vFirstSize = pFirst->GetSize();
            glm::vec2 vSecondSize = pSecond->GetSize();
            vSize = glm::vec2(vFirstSize.x + nSpacing + vSecondSize.x, std::max(vFirstSize.y, vSecondSize.y));
        }

        void SetOffset(glm::vec2 offset) override
        {
            this->vOffset = offset;
            pFirst->SetOffset(offset);
            pSecond->SetOffset(glm::vec2(offset.x + pFirst->GetSize().x + float(nSpacing), offset.y));

            if (pFirst->GetSize().y > pSecond->GetSize().y)
                pSecond->SetOffset(pSecond->vOffset + glm::vec2(0.0f, (pFirst->GetSize().y - pSecond->GetSize().y) * 0.5f));
            else
                pFirst->SetOffset(pFirst->vOffset + glm::vec2(0.0f, (pSecond->GetSize().y - pFirst->GetSize().y) * 0.5f));
        }
        void SetPosition(glm::vec2 pos) override
        {
            vPosition = pos;
            pFirst->vPosition = pos;
            pSecond->vPosition = pFirst->vPosition + glm::vec2(float(nSpacing), 0.0f);
        }

        virtual void Render(SpriteRenderer* pSpriteRenderer)
        {
            pFirst->Render(pSpriteRenderer);
            pSecond->Render(pSpriteRenderer);
        }
    };

    struct ControlHolder 
    {
        std::shared_ptr<Control> control;
        ControlType type;
        template<class T> T get() { return dynamic_cast<T>(control.get()); }
    };

    class ControlsManager
    {
    public:
        virtual ~ControlsManager()
        {
            Clear();
        }
        virtual ControlHolder& GetControl(std::string unique_name) 
        {
            if (control_indexes.count(unique_name) == 0)
                throw std::out_of_range("ControlsManager::GetControl(): Cannot find control with name '" + unique_name + "'.");
            return controls[control_indexes[unique_name]];
        }
        virtual void AddControl(std::string unique_name, std::shared_ptr<Control> control, ControlType type)
        {
            if (control_indexes.count(unique_name) != 0)
                throw std::invalid_argument("ControlsManager::GetControl(): Control with name '" + unique_name + "' already exists.");
            controls.push_back(ControlHolder{ control, type });
            control_indexes[unique_name] = controls.size() - 1;
        }
        virtual void RemoveControl(std::string unique_name) 
        { 
            if (control_indexes.count(unique_name) == 0)
                throw std::out_of_range("ControlsManager::RemoveControl(): Cannot find control with name '" + unique_name = "'.");
            size_t index = control_indexes[unique_name];
            controls.erase(controls.begin() + index);
        }
        virtual void Clear() { controls.clear(); controls.clear(); }

        template<class T> 
        T GetControl(std::string unique_name) { return GetControl(unique_name).get<T>(); }
    protected:
        std::unordered_map<std::string, size_t> control_indexes;
        std::vector<ControlHolder> controls;
    };

    enum class Gravity : int { left = 0, right = 1, center = 2 };

    // For now, Form is only a vertical linear layout.
    // Note: Position of all child controls are relative to their parent container (eg. form in this case).
    class Form : public ControlsManager, public IRenderable, public IObserver
    {
    public:
        AtlasTextRenderer* pTextRenderer = nullptr;
        int nSpacing = 10;

        Form(AtlasTextRenderer* text_renderer) : pTextRenderer(text_renderer) {}
        ~Form() { }

        Form& AddLabel(std::string unique_name, std::string label_text, glm::vec2 font_size, glm::vec3 color)
        {
            Label* label = new Label(label_text, compute_next_position(), font_size, color, pTextRenderer);
            label->sName = unique_name;
            AddControl(unique_name, std::shared_ptr<Control>(label), ControlType::label);
            return *this;
        }
        Form& AddPair(std::string unique_name, std::shared_ptr<Control> first, std::shared_ptr<Control> second)
        {
            Pair* pair = new Pair(first, second, compute_next_position());
            pair->sName = unique_name;
            AddControl(unique_name, std::shared_ptr<Control>(pair), ControlType::pair);
            return *this;
        }
        Form& MoveTo(glm::vec2 pos)
        {
            this->vPosition = pos;
            update_child_offsets();
            return *this;
        }
        void Render(SpriteRenderer* pRenderer)
        {
            for (auto& holder : controls)
                holder.control->Render(pRenderer);
        }
        void OnResize()
        {
            update_child_offsets();
        }
        Form& SetGravity(Gravity gravity)
        {
            this->gravity = gravity;
            update_total_size();
            update_child_positions();
            return *this;
        }

        // Overrides for ControlsManager.
        void AddControl(std::string unique_name, std::shared_ptr<Control> control, ControlType type)
        {
            ControlsManager::AddControl(unique_name, control, type);
            control->AddObserver(this);
            update_total_size();
        }
        void RemoveControl(std::string unique_name) override
        {
            ControlsManager::RemoveControl(unique_name);
            update_total_size();
            update_child_positions();
        }

        // Implementation of IObserver
        void OnNotify(IObserverSubject* obj, int message, void* args = nullptr)
        {
            if (message == MSG_SIZE_CHANGED)
            {
                update_total_size();
                update_child_positions();
            }
        }

    protected:
        Gravity gravity = Gravity::left;

        void update_child_positions()
        {
            glm::vec2 pos(0.0f, 0.0f);
            for (auto& holder : controls)
            {
                holder.control->vPosition = pos;
                pos.y += holder.control->GetSize().y + nSpacing;

                switch(this->gravity)
                {
                case Gravity::center: holder.control->SetPosition(glm::vec2((vSize.x - holder.control->GetSize().x) * 0.5f, holder.control->vPosition.y)); break;
                case Gravity::right: holder.control->SetPosition(glm::vec2(vSize.x - holder.control->GetSize().x, holder.control->vPosition.y)); break;
                default:
                    break;
                }
            }
        }
        glm::vec2 compute_next_position()
        {
            glm::vec2 pos(0.0f, 0.0f);
            for (auto& holder : controls)
                pos.y += holder.control->GetSize().y + nSpacing;
            return pos;
        }
        void update_total_size()
        {
            glm::vec2 size(0.0f);
            for (auto& holder : controls)
            {
                size.y += holder.control->GetSize().y + nSpacing;
                size.x = std::max(size.x, holder.control->GetSize().x);
            }
            size.y -= nSpacing;

            vSize = size;
        }
        void update_child_offsets()
        {
            for (auto& holder : controls)
                holder.control->SetOffset(this->vPosition);
        }
    };
};