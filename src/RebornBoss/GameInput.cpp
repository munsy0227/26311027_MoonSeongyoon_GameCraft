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
