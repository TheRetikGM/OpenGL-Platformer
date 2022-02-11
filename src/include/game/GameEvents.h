#pragma once

/*
* Identify events in game.
*/

#define INVALID_EVENT           0x0
#define PLAYER_HIT_SPIKES       0x1
#define REACHED_LEVEL_FINISH    0x2
#define PLAYER_JUMPED           0x3
#define PLAYER_WALL_JUMPED      0x4
#define PLAYER_LANDED           0x5
#define PLAYER_COLLIDE_COIN     0x6
#define PLAYER_LOST_LIFE        0x7
#define LEVEL_RESTARTED         0x8
#define PLAYER_DIED             0x9
#define PLAYER_REACHED_FINISH   0xA

struct Event 
{ 
    IObserverSubject* sender = nullptr;
    int message = 0x0; 
    void* args = nullptr;
};
inline bool operator==(const Event& a, const Event& b)
{
    return a.sender == b.sender
        && a.message == b.message
        && a.args == b.args;
}