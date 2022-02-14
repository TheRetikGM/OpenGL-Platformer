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
#define PLAYER_REACHED_FINISH   0xa
#define LEVEL_LOCKED_CHANGED    0xb
#define LEVEL_COMPLETED_CHANGED 0xc
#define LEVEL_PROGRESS_RESETED  0xd
#define LEVEL_LOADED            0xe
#define MAIN_MENU_LOADED        0xf

struct Event 
{ 
    IObserverSubject* sender = nullptr;
    int message = 0x0; 
    std::any args = nullptr;
};
inline bool operator==(const Event& a, const Event& b)
{
    return a.sender == b.sender
        && a.message == b.message;
}