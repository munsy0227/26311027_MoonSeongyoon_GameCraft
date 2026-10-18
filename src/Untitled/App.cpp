#include "App.h"
#include "GameInput.h"
#include "GameScene.h"

App::App()
{

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
	m_pGameInput = new GameInput;
	if (0 > m_pGameInput->Init())
	{
		Destroy();
		return -2;
	}

	//게임 씬 초기화
	m_pGameScene = new GameScene;
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
