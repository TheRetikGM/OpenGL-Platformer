#pragma once
#include <functional>
#include <algorithm>

class Timer
{
public:
    std::function<void(Timer*)> OnEnd = [](Timer* t) {};
    bool bEnded = true;

    Timer() {}
    virtual ~Timer() {}

    inline void Start(float duration, std::function<void(Timer*)> onEnd) { Restart(duration, OnEnd); }
    void Restart(float duration, std::function<void(Timer*)> onEnd)
    {
        OnEnd = onEnd;
        fDuration = duration;
        fCurrentDuration = 0.0f;
        bEnded = false;
    }
    void Update(float dt)
    {
        if (bEnded)
            return;

        fCurrentDuration += dt;
        if (fCurrentDuration >= fDuration)
        {
            OnEnd(this);
            bEnded = true;
        }
    }
protected:
    float fDuration = 0.0f;
    float fCurrentDuration = 0.0f;
};