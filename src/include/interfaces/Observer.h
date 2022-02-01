#pragma once

/*
* Easy and fast implementation of the observer pattern.
*/

class IObserverSubject;

class IObserver
{
public:
    virtual void OnNotify(IObserverSubject* obj, int message, void* args = nullptr) = 0;
};

class IObserverSubject
{
public:
    virtual void AddObserver(IObserver* o) = 0;
    virtual void RemoveObserver(IObserver* o) = 0;
protected:
    virtual void notify(int message, void* args = nullptr) = 0;
};