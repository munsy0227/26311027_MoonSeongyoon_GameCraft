#include "App.h"
#include "SceneBegin.h"
#include "SceneLobby.h"
#include "ScenePlay.h"
#include "ScenePlayResult.h"
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <windows.h>

#ifndef NDEBUG
void CheckGameplay();
#endif

App* g_pApp{};

App::App()
{
    g_pApp = this;
    m_sceneList.push_back(std::make_unique<SceneBegin>());
    m_sceneList.push_back(std::make_unique<SceneLobby>());
    m_sceneList.push_back(std::make_unique<ScenePlay>());
    m_sceneList.push_back(std::make_unique<ScenePlayResult>());
}

App::~App()
{
    Destroy();
    g_pApp = nullptr;
}

int App::Init()
{
    // 실행 파일을 직접 실행해도 같은 resources 폴더를 찾는다.
    std::vector<wchar_t> path(32768);
    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (length == 0 || length >= path.size())
    {
        std::fprintf(stderr, "Cannot locate the executable directory.\n");
        return -1;
    }
    std::error_code error;
    std::filesystem::current_path(std::filesystem::path(path.data()).parent_path(), error);
    if (error)
    {
        std::fprintf(stderr, "Cannot set the executable directory: %s\n", error.message().c_str());
        return -1;
    }
#ifndef NDEBUG
    CheckGameplay();
    std::puts("Gameplay checks passed.");
#endif
    if (m_graphicEngine.Init(m_gameWin) < 0)
        return -1;
    m_graphicsReady = true;
    m_graphicEngine.SetColor(0xFF0E111A);
    if (m_gameInput.Init() < 0 || m_resources.Init() < 0)
        return -1;
    m_gameTime.Init();
    m_pGameScene = m_sceneList[SCENE_BEGIN].get();
    return m_pGameScene->Init();
}

void App::Destroy()
{
    if (m_pGameScene)
        m_pGameScene->Destroy();
    m_pGameScene = nullptr;
    m_sceneList.clear();
    if (m_graphicsReady)
    {
        m_resources.Destroy();
        m_graphicEngine.Destroy();
        m_graphicsReady = false;
    }
}

int App::Update()
{
    if (m_sceneChanged)
    {
        m_pGameScene->Destroy();
        m_sceneCurrent = m_sceneNext;
        m_sceneChanged = false;
        m_pGameScene = m_sceneList[static_cast<size_t>(m_sceneCurrent)].get();
        if (m_pGameScene->Init() < 0)
            return m_loopError = -1;
    }
    m_gameTime.Update();
    if (m_gameInput.Update() < 0)
        return m_loopError = -1;
    const int result = m_pGameScene->Update();
    if (result < 0)
        m_loopError = result;
    return result;
}

int App::Render()
{
    const int result = m_pGameScene->Render();
    if (result < 0)
        m_loopError = result;
    return result;
}

int App::Run()
{
    // glc2d 루프는 콜백 오류로 끝나도 0을 반환하므로 오류를 보관한다.
    const int result = m_graphicEngine.Run();
    return m_loopError < 0 ? m_loopError : result;
}

void App::ChangeScene(GAME_PHASE nextScene)
{
    if (nextScene >= SCENE_BEGIN && static_cast<size_t>(nextScene) < m_sceneList.size())
    {
        m_sceneNext = nextScene;
        m_sceneChanged = true;
    }
}

float App::DeltaSeconds() const
{
    return std::clamp(static_cast<float>(m_gameTime.deltaTime) / 1000.0F, 0.0F, 0.05F);
}
