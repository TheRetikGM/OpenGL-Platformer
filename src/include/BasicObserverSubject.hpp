#pragma once
#include "interfaces/Observer.h"
#include <list>
#include <algorithm>

class BasicObserverSubject : public IObserverSubject
{
public:
    virtual void AddObserver(IObserver* o) {
        if (std::find(observers.begin(), observers.end(), o) == observers.end())
            observers.emplace_back(o);
    }
    virtual void RemoveObserver(IObserver* o) {
        observers.remove(o);
    }
protected:
    std::list<IObserver*> observers;

    virtual void notify(int message, std::any args = nullptr)
    {
        for (IObserver* o : observers)
            o->OnNotify(this, message, args);
    }
};