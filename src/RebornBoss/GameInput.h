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
    bool ConfirmPressed() const;
    bool CancelPressed() const;
    float MovementX() const;
    float MovementY() const;
    bool ShotHeld() const;
    bool BombPressed() const;
    bool SlowHeld() const;
    bool SkipDialogueHeld() const;
    int MenuDirection() const;
    bool IsNewClassic() const { return m_newClassic; }
    void ToggleControlPreset() { m_newClassic = !m_newClassic; }

private:
    bool m_newClassic{};
};

extern GameInput* GetGameInput();
