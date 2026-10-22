#include "GameInput.h"

static GameInput* g_input;
GameInput* GetGameInput()
{
	return g_input;
}

int GameInput::Init()
{
	g_input = this;
	return 0;
}

int GameInput::Update()
{
	return 0;
}

VEC3i GameInput::MousePosition() const
{
	return g2_GetMouse();
}
int GameInput::MouseDelta() const
{
	auto r = g2_GetMouse().z;
	return r;
}
int GameInput::GetKeyState(KEYCODE keyCode) const
{
	auto keys = g2_GetKeyboard();										// Å°º¸µå(0~255)
	return keys[keyCode];
}
