#pragma once
#include <memory>
#include <vector>
#include "GameTime.h"
#include "GameInput.h"
#include "GraphicEngine.h"
#include "GameScene.h"
#include "GameResources.h"

struct StageResult
{
    bool cleared{};
    int score{};
    float elapsedSeconds{};
};

class App
{
public:
    App();
    ~App();
    int Init();
    void Destroy();
    int Update();
    int Render();
    int Run();
    void ChangeScene(GAME_PHASE nextScene);
    float DeltaSeconds() const;
    GameResources& Resources() { return m_resources; }
    const StageResult& Result() const { return m_result; }
    void SetResult(StageResult result) { m_result = result; }

private:
    GraphicEngine::GAME_WINDOW m_gameWin{120, 50, 960, 600, "RebornBoss", true, 1.0F};
    GraphicEngine m_graphicEngine{};
    GameTime m_gameTime{};
    GameInput m_gameInput{};
    GameResources m_resources{};
    std::vector<std::unique_ptr<GameScene>> m_sceneList;
    GameScene* m_pGameScene{};
    StageResult m_result{};
    int m_loopError{};
    bool m_graphicsReady{};
    bool m_sceneChanged{};
    GAME_PHASE m_sceneCurrent{SCENE_BEGIN};
    GAME_PHASE m_sceneNext{SCENE_BEGIN};
};

extern App* g_pApp;
