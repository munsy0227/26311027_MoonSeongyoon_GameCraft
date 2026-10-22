#include "SceneBegin.h"
#include "GameInput.h"
#include "App.h"
SceneBegin::SceneBegin()
{
	m_message = "SceneBegin";
}

int SceneBegin::Update()
{
	auto keyEvent = GetGameInput()->GetKeyState(VK_RIGHT);
	if (keyEvent == EInput::EINPUT_DOWN)
	{
		// next window
		g_pApp->ChangeScene(GAME_PHASE::SCENE_LOBBY);
	}

	return 0;
}
