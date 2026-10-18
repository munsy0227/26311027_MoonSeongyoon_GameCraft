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
    // GameTime 관련 초기화
    m_gameTime.Init();

    // Glc2d 초기화

    // Input 초기화
    m_pGameInput = new GameInput;
    if (0 > m_pGameInput->Init())
    {
        delete m_pGameInput;
        return -1;
    }

    // GameScene 초기화
    m_pGameScene = new GameScene;
    if (0 > m_pGameInput->Init())
    {
        Destroy();
        return -2;
    }
        

    return 0;
}

int App::Destroy()
{
    if (m_pGameInput) {
        delete m_pGameInput; m_pGameInput = nullptr;
    }

    if (m_pGameInput) {
        delete m_pGameScene; m_pGameScene = nullptr;
    }
    return 0;
}

int App::Update()
{
    // GameTime 관련 업데이트

    // Input 업데이트
    
    // GameScene 업데이트

    return 0;
}

int App::Render()
{
    // GameScene 렌더링

    return 0;
}

int App::Run()
{
    int hr = 0;
    while (true)
    {
        // 게임 업데이트
        if (0 > this->Update())
        {
            hr = -1;
            break;
        }

        // 게임 렌더링
        if (0 > this->Render())
        {
            hr = -2;
            break;
        }
    }

    this->Destroy();
    return 0;
}
