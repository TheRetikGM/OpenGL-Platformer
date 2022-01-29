#pragma once
#include "interfaces/Observer.h"
#include <list>

class BasicObserverSubject : public IObserverSubject
{
public:
    void AddObserver(IObserver* o) {
        observers.emplace_back(o);
    }
    void RemoveObserver(IObserver* o) {
        observers.remove(o);
    }
protected:
    std::list<IObserver*> observers;

    void notify(int message)
    {
        for (IObserver* o : observers)
            o->OnNotify(this, message);
    }
};