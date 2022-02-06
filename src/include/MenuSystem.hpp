#pragma once
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <list>
#include <any>
#include "texture.h"
#include "shader.h"
#include "AtlasTextRenderer.hpp"
#include "sprite_renderer.h"
#include "Helper.hpp"   // Only for debugging. Not needed.

/*
*  Menu system inspired by https://www.youtube.com/watch?v=jde1Jq5dF0E&list=WL&index=13
*
*  |-----------Panel-----------|
*  | |---------Table---------| |
*  | | |--A--|      |--E--|  | |
*  | | |--B--|      |--F--|  | |
*  | | |--C--|      |--G-------------\         |-----Panel-----|
*  | | |--D--|      |--H--|  | |      \------> |  |--Table--|  |
*  | |_______________________| |               |_______________| 
*  |___________________________|                                \-------> | etc. | 
*  
*  Note:
*     Panel == MenuObject
*/


namespace MenuSystem
{
    // Handles drawing of the menu.
    class MenuRenderer;
    // Handles input and navigating the menu.
    class MenuManager;
    // Acts as a panel, which contains tables.
    class MenuObject
    {
    public:
        MenuObject()
        {
            sName = "root";
        }
        MenuObject(const std::string& name)
        {
            sName = name;
        }

        // Sets the table dimensions.
        MenuObject& SetTable(int columns, int rows)
        {
            vCellTable.x = columns;
            vCellTable.y = rows;

            return *this;
        }
        MenuObject& SetID(int id)
        {
            nID = id;
            return *this;
        }
        // Size of one patch in pixels.
        // Build needed.
        MenuObject& SetPatchSize(glm::ivec2 size)
        {
            this->vPatchSize = size;
            return *this;
        }
        int GetID() { return nID; }
        MenuObject& Enable(bool b)
        {
            bEnabled = b;
            return *this;
        }
        bool Enabled() const { return bEnabled; }
        const std::string& GetName() const
        {
            return sName;
        }
        // Total size of the object window in pixels.
        glm::ivec2 GetTotalSize(float scale = 1.0f) const 
        { 
            return glm::ivec2(glm::vec2(vSizeInPatches * vPatchSize) * scale); 
        }
        // Size required to display the name of the menu object.
        // For now, cells are simply one line strings.
        glm::ivec2 GetSize(AtlasTextRenderer* pTextRenderer) const
        {
            glm::vec2 vNameSize = pTextRenderer->GetStringSize(sName);

            vNameSize *= vTextScale;
            float width = vNameSize.x / vPatchSize.x + (((int(vNameSize.x) % vPatchSize.x) != 0) ? 1 : 0);

            return glm::ivec2(width, 1);
        }
        bool HasChildren() const
        {
            return !items.empty();
        }

        MenuObject& operator[] (const std::string& name)
        {
            if (itemsPointer.count(name) == 0)
            {
                itemsPointer[name] = items.size();
                items.push_back(MenuObject(name));
            }

            return items[itemsPointer[name]];
        }
        MenuObject& at(const std::string& name)
        {
            return operator[](name);
        }
        MenuObject& SetTextColor(glm::vec3 color) {
            this->vTextColor = color; 
            return *this; 
        }
        MenuObject& SetDisabledTextColor(glm::vec3 color) { this->vTextColor_disabled = color; return *this; }
        // Populates the menuobject internal variables with relevant sizes for drawing.
        // Required for usage.
        void Build(AtlasTextRenderer* pTextRenderer)
        {
            this->pTextRenderer = pTextRenderer;
            this->vTextScale = get_text_scale();
            this->vTextOffset = get_text_offset();

            // Recursively build all children, so they can determine their size, use
            // that size to indicate cell sizes if this object contains more than
            // one item.
            for (auto& object : items)
            {
                if (object.HasChildren())
                    object.Build(pTextRenderer);
                else {
                    object.vTextScale = get_text_scale();
                    object.vTextOffset = get_text_offset();
                }

                // Longest child name determines cell width (in patches).
                vCellSize.x = std::max(vCellSize.x, object.GetSize(pTextRenderer).x);
                vCellSize.y = std::max(vCellSize.y, object.GetSize(pTextRenderer).y);
            }

            // Adjust size of this object (in patches) if it were rendered as a panel.
            vSizeInPatches.x = vCellTable.x * vCellSize.x + (vCellTable.x - 1) * vCellPadding.x + 2;
            vSizeInPatches.y = vCellTable.y * vCellSize.y + (vCellTable.y - 1) * vCellPadding.y + 2;

            // How many rows this object has to hold.
            nTotalRows = (items.size() / vCellTable.x) + (((items.size() % vCellTable.x) > 0) ? 1 : 0);
        }

        // Clamps cursor to the boundaries of the item list.
        // Ensures, that we cannot point to invalid location in the item list.
        void ClampCursor()
        {
            // Find item in children.
            nCursorItem = vCellCursor.y * vCellTable.x + vCellCursor.x;

            // Clamp the cursor. Has the effect of moving the cursor to
            // the left-most item in the table row.
            if (nCursorItem >= int(items.size()))
            {
                vCellCursor.y = (items.size() / vCellTable.x);
                vCellCursor.x = (items.size() % vCellTable.x) - 1;
                nCursorItem = items.size() - 1;
            }
        }

        // Moves the cursor up (scroll if needed).
        void OnUp()
        {
            vCellCursor.y--;
            if (vCellCursor.y < 0)
                vCellCursor.y = 0;
            
            if (vCellCursor.y < nTopVisibleRow)
                nTopVisibleRow--;
            
            ClampCursor();
        }
        // Moves the cursor down (scroll if needed).
        void OnDown()
        {
            vCellCursor.y++;
            
            if (vCellCursor.y == nTotalRows) 
                vCellCursor.y = nTotalRows - 1;
            
            if (vCellCursor.y > (nTopVisibleRow + vCellTable.y - 1))
            {
                nTopVisibleRow++;
                if (nTopVisibleRow > (nTotalRows - vCellTable.y))
                    nTopVisibleRow = nTotalRows - vCellTable.y;
            }

            ClampCursor();
        }
        // Moves the cursor left.
        void OnLeft()
        {
            vCellCursor.x--;
            if (vCellCursor.x < 0)
                vCellCursor.x = 0;
            ClampCursor();
        }
        // Moves the cursor right.
        void OnRight()
        {
            vCellCursor.x++;
            if (vCellCursor.x > vCellTable.x - 1)
                vCellCursor.x = vCellTable.x - 1;
            ClampCursor();
        }

        MenuObject& SetCustomData(std::any data) { custom_data = data; return *this; }
        std::any& GetCustomData() { return custom_data; }
        template<class type> type GetCustomData() { return std::any_cast<type>(custom_data); }

        // Handles the on confirm event.
        // If item contains another menu, then the pointer to the submenu is returned.
        // Else pointer to this item is returned.
        MenuObject* OnConfirm()
        {
            if (items[nCursorItem].HasChildren())
                return &items[nCursorItem];
            else
                return this;
        }

        MenuObject* GetSelectedItem()
        {
            return &items[nCursorItem];
        }


    protected:
        glm::vec3 vTextColor = glm::vec3(1.0f, 1.0f, 1.0f);   // Blue
        glm::vec3 vTextColor_disabled = glm::vec3(0.2f, 0.2f, 0.2f);  // Dark gray
        std::string sName = "";
        std::any custom_data = 0;
        bool bEnabled = true;
        // ID for event identification.
        int nID = -1;
        // Number of rows, that make up the table.
        int nTotalRows = 0;
        // Where in the table to start drawing from.
        int nTopVisibleRow = 0;

        // Size of the table in cells.
        glm::ivec2 vCellTable = glm::ivec2(1, 0);
        // Size of one cell in patches.
        glm::ivec2 vCellSize = glm::ivec2(0, 0);    // Is calcualted in the Build() fucntion.
        // Padding between table cells in pathes.
        glm::ivec2 vCellPadding = glm::ivec2(1, 0);

        // Size of one patch (for rendering **NOT** for sampling).
        glm::ivec2 vPatchSize = glm::ivec2(30.0f);
        // Size of this menuobject in patches.
        glm::ivec2 vSizeInPatches = glm::vec2(0, 0);
        glm::vec2 vTextScale = glm::vec2(1.0f);
        // Offset of the text in pixels.
        glm::vec2 vTextOffset = glm::vec2(0.0f, 0.0f);

        // Stored items.
        std::unordered_map<std::string, size_t> itemsPointer;
        std::vector<MenuObject> items;

        // Cell pointed to by cursor.
        glm::ivec2 vCellCursor = glm::ivec2(0, 0);
        // Index of the item pointed to by cursor.
        int nCursorItem = 0;
        // Cursor position in screen space.
        glm::ivec2 vCursorPos = glm::ivec2(0, 0);

        // Text renderer, for which this menu was built.
        AtlasTextRenderer* pTextRenderer = nullptr;

        // Calculate required text scale with the current size
        // of one patch and current text renderer in use.
        glm::vec2 get_text_scale()
        {
            // Take only 60% of the calculated scale for some spacing between rows.
            const float bias = 0.6f;
            return (glm::vec2(vPatchSize) / pTextRenderer->vCharSize) * bias;
        }
        // Calculate required offset for the text to be in the vertical center of the patch row.
        glm::vec2 get_text_offset()
        {
            float offset = 0.4f * vPatchSize.y;
            return glm::vec2(0.0f, offset);
        }

        // Manager and renderer classes can access protected members.
        friend class MenuRenderer;
        friend class MenuManager;
    };

    class MenuManager
    {
    public:
        MenuManager() {}

        void Open(MenuObject* mo)
        {
            Close();
            panels.push_back(mo);
        }
        void Close()
        {
            panels.clear();
        }
        void OnUp()
        {
            if (!panels.empty())
                panels.back()->OnUp();
        }
        void OnDown()
        {
            if (!panels.empty())
                panels.back()->OnDown();
        }
        void OnLeft()
        {
            if (!panels.empty())
                panels.back()->OnLeft();
        }
        void OnRight()
        {
            if (!panels.empty())
                panels.back()->OnRight();
        }
        void OnBack()
        {
            if (panels.size() == 1 && !bCloseOnBack)
                return;
            if (!panels.empty())
                panels.pop_back();
        }
        inline bool MenuClosed() { return panels.size() == 0; }
        inline const MenuObject& First() 
        {
            if (panels.empty())
                throw std::out_of_range("MenuManager::First(): Menu is not opened.");
            return *panels.front(); 
        }

        MenuObject* OnConfirm()
        {
            if (panels.empty())
                return nullptr;
            MenuObject* next = panels.back()->OnConfirm();

            if (next == panels.back()) 
            {
                if (panels.back()->GetSelectedItem()->Enabled())
                    return panels.back()->GetSelectedItem();
            }
            else
            {
                if (next->Enabled())
                    panels.push_back(next);
            }

            return nullptr;
        }
        MenuManager& CloseOnBack(bool b) { bCloseOnBack = b; return *this; }

    protected:
        std::list<MenuObject*> panels;
        bool bCloseOnBack = true;

        friend class MenuRenderer;
    };

    class MenuRenderer
    {
    public:
        SpriteRenderer* pSpriteRenderer;
        // Size of one patch in pixels.
        int nPatchSize = 16;

        MenuRenderer(SpriteRenderer* sr) : pSpriteRenderer(sr) {}

        void DrawObject(MenuObject& menu, Texture2D& pTexGFX, glm::ivec2 vScreenOffset, float scale = 1.0f)
        {
            // ===== Draw panel ===== //
            glm::ivec2 vPatchPos = glm::ivec2(0, 0);
            for (vPatchPos.x = 0; vPatchPos.x < menu.vSizeInPatches.x; vPatchPos.x++)
            {
                for (vPatchPos.y = 0; vPatchPos.y < menu.vSizeInPatches.y; vPatchPos.y++)
                {
                    // Determine position in screen space.
                    glm::vec2 vScreenLocation = glm::vec2(vPatchPos * menu.vPatchSize) * scale + glm::vec2(vScreenOffset);

                    // Calculate which patch is needed.
                    glm::ivec2 vSourcePatch(0);
                    if (vPatchPos.x > 0)
                        vSourcePatch.x = 1;
                    if (vPatchPos.x == menu.vSizeInPatches.x - 1)
                        vSourcePatch.x = 2;
                    if (vPatchPos.y > 0)
                        vSourcePatch.y = 1;
                    if (vPatchPos.y == menu.vSizeInPatches.y - 1)
                        vSourcePatch.y = 2;

                    pSpriteRenderer->DrawPartialSprite(pTexGFX, vSourcePatch * nPatchSize, glm::vec2(nPatchSize), vScreenLocation, glm::vec2(menu.vPatchSize) * scale);
                }
            }

            // ===== Draw panel contents ===== //
            glm::ivec2 vCell(0, 0);
            vPatchPos = glm::ivec2(1, 1);

            // Work out visible items.
            int nTopLeftItem = menu.nTopVisibleRow * menu.vCellTable.x;
            int nBottomRightItem = menu.vCellTable.y * menu.vCellTable.x + nTopLeftItem;

            // Clamp to size of child item vector.
            nBottomRightItem = std::min(int(menu.items.size()), nBottomRightItem);
            int nVisibleItems = nBottomRightItem - nTopLeftItem;

            // Draw scroll markers (if required).
            if (menu.nTopVisibleRow > 0)
            {
                vPatchPos = glm::ivec2(menu.vSizeInPatches.x - 2, 0);
                glm::vec2 vScreenLocation = glm::vec2(vPatchPos * menu.vPatchSize) * scale + glm::vec2(vScreenOffset);
                glm::ivec2 vSourcePatch(3, 0);
                pSpriteRenderer->DrawPartialSprite(pTexGFX, vSourcePatch * nPatchSize, glm::vec2(nPatchSize), vScreenLocation, glm::vec2(menu.vPatchSize) * scale);
            }
            if ((menu.nTotalRows - menu.nTopVisibleRow) > menu.vCellTable.y)
            {
                vPatchPos = menu.vSizeInPatches - glm::ivec2(2, 1);
                glm::vec2 vScreenLocation = glm::vec2(vPatchPos * menu.vPatchSize) * scale + glm::vec2(vScreenOffset);
                glm::ivec2 vSourcePatch(3, 2);
                pSpriteRenderer->DrawPartialSprite(pTexGFX, vSourcePatch * nPatchSize, glm::vec2(nPatchSize), vScreenLocation, glm::vec2(menu.vPatchSize) * scale);
            }

            // Draw visible items.
            for (int i = 0; i < nVisibleItems; i++)
            {
                vCell.x = i % menu.vCellTable.x;
                vCell.y = i / menu.vCellTable.x;

                // Patch location (including border offset and padding) in patches local to the table.
                vPatchPos.x = vCell.x * (menu.vCellSize.x + menu.vCellPadding.x) + 1;
                vPatchPos.y = vCell.y * (menu.vCellSize.y + menu.vCellPadding.y) + 1;

                // Actual screen location in pixels.
                glm::vec2 vScreenLocation = glm::vec2(vPatchPos * menu.vPatchSize) * scale + glm::vec2(vScreenOffset);

                // Draw item header.
                menu.pTextRenderer->RenderText(pSpriteRenderer, menu.items[nTopLeftItem + i].sName
                                             , vScreenLocation + menu.vTextOffset
                                             , menu.vTextScale * scale
                                             , menu.items[nTopLeftItem + i].bEnabled ? menu.items[nTopLeftItem + i].vTextColor : menu.items[nTopLeftItem + i].vTextColor_disabled
                );

                if (menu.items[nTopLeftItem + i].HasChildren())
                {
                    // Display indicator that panel has a sub panel.
                    vPatchPos.x = vCell.x * (menu.vCellSize.x + menu.vCellPadding.x) + 1 + menu.vCellSize.x;
                    vPatchPos.y = vCell.y * (menu.vCellSize.y + menu.vCellPadding.y) + 1;
                    glm::ivec2 vSourcePatch(3, 1);
                    vScreenLocation = glm::vec2(vPatchPos * menu.vPatchSize) * scale + glm::vec2(vScreenOffset);
                    pSpriteRenderer->DrawPartialSprite(pTexGFX, vSourcePatch * nPatchSize, glm::vec2(nPatchSize), vScreenLocation, glm::vec2(menu.vPatchSize) * scale);
                }

                // Calculate cursor position in screen space in case system draws it.
                menu.vCursorPos.x = float((menu.vCellCursor.x * (menu.vCellSize.x + menu.vCellPadding.x)) * menu.vPatchSize.x) * scale + vScreenOffset.x - float(menu.vPatchSize.x) * scale;
                menu.vCursorPos.y = (((menu.vCellCursor.y - menu.nTopVisibleRow) * (menu.vCellSize.y + menu.vCellPadding.y)) * menu.vPatchSize.y + menu.vPatchSize.y * 0.5f) * scale + vScreenOffset.y;
            }
        }
        void Draw(MenuManager& manager, Texture2D& sprGFX, glm::ivec2 vScreenOffset, float scale = 1.0f)
        {
            if (manager.panels.empty())
                return;
            
            // Draw visible menu system.
            for (auto& p : manager.panels)
            {
                DrawObject(*p, sprGFX, vScreenOffset, scale);
                vScreenOffset += glm::ivec2(50, 30);
            }

            // Draw cursor.
            pSpriteRenderer->DrawPartialSprite(sprGFX, glm::ivec2(4, 0) * nPatchSize, glm::ivec2(32), manager.panels.back()->vCursorPos, glm::vec2(manager.panels.back()->vPatchSize * 2) * scale);
        }
    };
}