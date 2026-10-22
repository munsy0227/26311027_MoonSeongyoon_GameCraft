#include "App.h"
#include "GameInput.h"
#include "GameScene.h"
#include "SceneBegin.h"
#include "SceneLobby.h"
#include "ScenePlay.h"
#include "ScenePlayResult.h"

App* g_pApp;

App::App()
{
	g_pApp = this;
	m_pGameInput = new GameInput;

	m_sceneList.push_back(new SceneBegin);
	m_sceneList.push_back(new SceneLobby);
	m_sceneList.push_back(new ScenePlay);
	m_sceneList.push_back(new ScenePlayResult);

	m_pGameScene = m_sceneList[m_sceneCurrent];
}

App::~App()
{

}

int App::Init()
{
	// Glc2d 초기화
	if (0 > m_graphicEngine.Init(m_gameWin))
		return -1;

	m_graphicEngine.SetColor(0xFF006699);

	// GameTime 초기화
	m_gameTime.Init();

	// Input 초기화
	if (0 > m_pGameInput->Init())
	{
		Destroy();
		return -2;
	}

	//게임 씬 초기화
	if(0> m_pGameScene->Init())
	{
		Destroy();
		return -3;
	}

	return 0;
}

void App::Destroy()
{
	if (m_pGameInput) {
		delete m_pGameInput; m_pGameInput = nullptr;
	}
	if (m_pGameScene) {
		delete m_pGameScene; m_pGameScene = nullptr;
	}
}

int  App::Update()
{
	// update change scene
	if (m_sceneChanged)
	{
		m_pGameScene->Destroy();			// 현재의 게임 씬 해제

		m_sceneChanged = false;
		m_sceneCurrent = m_sceneNext;		// 다음 씬을 현재 씬으로
		m_pGameScene   = m_sceneList[m_sceneCurrent];

		m_pGameScene->Init();				// 설정된 현재 씬을 초기화
	}

	// GameTime 업데이트
	m_gameTime.Update();

	// Input 업데이트
	if (0 > m_pGameInput->Update())
	{
		return -1;
	}

	// GameScene 업데이트
	if(0>m_pGameScene->Update())
	{
		return -1;
	}

	return 0;
}


int  App::Render()
{
	// GameScene 렌더링
	if (0 > m_pGameScene->Render())
	{
		return -1;
	}

	return 0;
}

int App::Run()
{
	m_graphicEngine.Run();

	this->Destroy();
	return 0;
}

void App::ChangeScene(GAME_PHASE nextScene)
{
	m_sceneChanged = true;
	m_sceneNext = nextScene;
}
