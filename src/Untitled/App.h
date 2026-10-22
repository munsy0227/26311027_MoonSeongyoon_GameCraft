#pragma once
#include <vector>
#include "GameTime.h"
#include "GraphicEngine.h"
#include "GameScene.h"

class GameInput;
class GameScene;

class App
{
public:
	App();
	virtual ~App();

	virtual int Init();
	virtual void Destroy();
	virtual int  Update();
	virtual int  Render();

	int Run();

public:
	void ChangeScene(GAME_PHASE nextScene);

protected:
	GraphicEngine::GAME_WINDOW m_gameWin
	{
		120, 50
		, 960, 600
		, "GameTitle"
		, true
		, 1.4F
	};
	GraphicEngine m_graphicEngine{};
	GameTime	m_gameTime	 {};
	GameInput*	m_pGameInput {};

	std::vector<GameScene*> m_sceneList;
	GameScene* m_pGameScene		{};	// 현재의 게임씬
	bool       m_sceneChanged	{};
	GAME_PHASE m_sceneCurrent	{SCENE_BEGIN};
	GAME_PHASE m_sceneNext		{};
};

extern App* g_pApp;
