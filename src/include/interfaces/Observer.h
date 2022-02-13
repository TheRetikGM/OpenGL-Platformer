#pragma once
#include <any>

/*
* Easy and fast implementation of the observer pattern.
*/

class IObserverSubject;

class IObserver
{
public:
    virtual void OnNotify(IObserverSubject* obj, int message, std::any args = nullptr) = 0;
};

class IObserverSubject
{
public:
    virtual void AddObserver(IObserver* o) = 0;
    virtual void RemoveObserver(IObserver* o) = 0;
protected:
    virtual void notify(int message, std::any args = nullptr) = 0;
};