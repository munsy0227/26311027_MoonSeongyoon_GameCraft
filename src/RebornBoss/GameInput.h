#pragma once
#include <glc2d.h>

class GameInput
{
public:
    int Init();
    int Update();
    VEC3i MousePosition() const;
    int MouseDelta() const;
    int GetKeyState(KEYCODE keyCode) const;
    bool Pressed(KEYCODE keyCode) const;
    bool Held(KEYCODE keyCode) const;
};

extern GameInput* GetGameInput();
