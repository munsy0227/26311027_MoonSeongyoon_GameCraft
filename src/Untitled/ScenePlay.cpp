#include "ScenePlay.h"
#include "GameInput.h"
#include "App.h"

ScenePlay::ScenePlay()
{
	m_message = "Scene Play";
}

int ScenePlay::Update()
{
	auto keyEvent = GetGameInput()->GetKeyState(VK_RIGHT);
	if (keyEvent == EInput::EINPUT_DOWN)
	{
		// next window
		g_pApp->ChangeScene(GAME_PHASE::SCENE_PLAYRESULT);
	}

	return 0;
}
