#include "GameInput.h"

static GameInput* g_input{};
GameInput* GetGameInput() { return g_input; }
int GameInput::Init() { g_input = this; return 0; }
int GameInput::Update() { return 0; }
VEC3i GameInput::MousePosition() const { return g2_GetMouse(); }
int GameInput::MouseDelta() const { return g2_GetMouse().z; }
int GameInput::GetKeyState(KEYCODE keyCode) const { return g2_GetKeyboard()[keyCode]; }
bool GameInput::Pressed(KEYCODE keyCode) const
{
    return GetKeyState(keyCode) == EInput::EINPUT_DOWN;
}
bool GameInput::Held(KEYCODE keyCode) const
{
    const auto state = GetKeyState(keyCode);
    return state == EInput::EINPUT_DOWN || state == EInput::EINPUT_PRESS;
}

bool GameInput::ConfirmPressed() const
{
    return Pressed('Z') || (m_newClassic && (Pressed('J') || Pressed(VK_RETURN)));
}

bool GameInput::CancelPressed() const
{
    return Pressed('X') || Pressed(VK_ESCAPE) || (m_newClassic && Pressed('K'));
}

float GameInput::MovementX() const
{
    const bool right = Held(VK_RIGHT) || Held(VK_NUMPAD6) || Held(VK_NUMPAD3) || Held(VK_NUMPAD9)
                    || (m_newClassic && Held('D'));
    const bool left = Held(VK_LEFT) || Held(VK_NUMPAD4) || Held(VK_NUMPAD1) || Held(VK_NUMPAD7)
                   || (m_newClassic && Held('A'));
    return static_cast<float>(right) - static_cast<float>(left);
}

float GameInput::MovementY() const
{
    const bool down = Held(VK_DOWN) || Held(VK_NUMPAD2) || Held(VK_NUMPAD1) || Held(VK_NUMPAD3)
                   || (m_newClassic && Held('S'));
    const bool up = Held(VK_UP) || Held(VK_NUMPAD8) || Held(VK_NUMPAD7) || Held(VK_NUMPAD9)
                 || (m_newClassic && Held('W'));
    return static_cast<float>(down) - static_cast<float>(up);
}

bool GameInput::ShotHeld() const
{
    return Held('Z') || (m_newClassic && Held('J'));
}

bool GameInput::BombPressed() const
{
    return Pressed('X') || (m_newClassic && Pressed('K'));
}

bool GameInput::SlowHeld() const
{
    return Held(VK_SHIFT) || (m_newClassic && (Held(VK_LSHIFT) || Held(VK_SPACE) || Held('L')));
}

bool GameInput::SkipDialogueHeld() const
{
    return m_newClassic && (Held(VK_CONTROL) || Held(VK_LCONTROL) || Held(VK_OEM_1));
}

int GameInput::MenuDirection() const
{
    const bool next = Pressed(VK_DOWN) || Pressed(VK_RIGHT) || Pressed(VK_NUMPAD2) || Pressed(VK_NUMPAD6)
                   || (m_newClassic && (Pressed('S') || Pressed('D')));
    const bool previous = Pressed(VK_UP) || Pressed(VK_LEFT) || Pressed(VK_NUMPAD8) || Pressed(VK_NUMPAD4)
                       || (m_newClassic && (Pressed('W') || Pressed('A')));
    return static_cast<int>(next) - static_cast<int>(previous);
}
