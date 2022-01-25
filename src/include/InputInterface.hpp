#pragma once
#include <GLFW/glfw3.h>

/*
*  Simple keyboard interface wrapper for checking key presses.
*/
class InputInterface
{
public:
    InputInterface(bool* pKeys, bool* pKeysProcessed) : pKeys(pKeys), pKeysProcessed(pKeysProcessed) {}

    // Checks ifs key was pressed (will return true only once until it is released).
    inline bool Pressed(int key)
    {
        if (pKeys[key] && !pKeysProcessed[key])
        {
            pKeysProcessed[key] = true;
            return true;
        }
        return false;
    }
    // Checks if key is being held.
    inline bool Held(int key)
    {
        return pKeys[key];
    }

protected:
    bool* pKeys;
    bool* pKeysProcessed;
};