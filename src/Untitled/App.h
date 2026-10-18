#pragma once

#include "GameTime.h"

class GameInput;
class GameScene;

class App
{
public:
	App();
	virtual ~App();

	int Init();
	int Destroy();
	int Update();
	int Render();

	int Run();
protected:
	GameTime    m_gameTime   {};
	GameInput*  m_pGameInput {};
	GameScene*  m_pGameScene {};
};

extern App* g_pApp;