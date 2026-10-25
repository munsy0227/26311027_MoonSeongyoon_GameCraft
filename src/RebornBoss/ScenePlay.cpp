#include "ScenePlay.h"
#include "GameInput.h"
#include "App.h"
#include <algorithm>
#include <cstdio>

int ScenePlay::Init()
{
    m_player = GamePlayer{};
    m_bullets.Clear();
    m_mobs.clear();
    m_elapsedSeconds = 0.0F;
    m_spawnTimer = 0.75F;
    m_spawnCount = m_selected = 0;
    m_paused = false;
    g_pApp->Resources().PlayBgm(SoundId::Forest);
    return 0;
}

int ScenePlay::Destroy()
{
    m_bullets.Clear();
    m_mobs.clear();
    return 0;
}

bool ScenePlay::Tick(float seconds, float directionX, float directionY, bool slow, bool shooting)
{
    if (m_paused)
        return false;
    m_elapsedSeconds += seconds;
    m_player.Update(seconds, directionX, directionY, slow, shooting);
    const bool fired = m_bullets.Update(seconds, shooting, m_player.X(), m_player.Y());
    m_spawnTimer -= seconds;
    constexpr float positions[] = {96.0F, 240.0F, 384.0F};
    while (m_spawnTimer <= 0.0F)
    {
        m_mobs.emplace_back(positions[m_spawnCount % 3]);
        m_spawnCount = (m_spawnCount + 1) % 3;
        m_spawnTimer += 1.5F;
    }
    for (auto& mob : m_mobs)
        mob.Update(seconds);
    std::erase_if(m_mobs, [](const GameMob& mob) { return mob.Outside(); });
    return fired;
}

int ScenePlay::Update()
{
    const auto* input = GetGameInput();
    if (input->Pressed(VK_ESCAPE))
    {
        m_paused = !m_paused;
        m_selected = 0;
        return 0;
    }
    if (m_paused)
    {
        MoveSelection(m_selected, 2);
        if (input->Pressed(VK_RETURN))
        {
            g_pApp->Resources().PlaySound(SoundId::MenuConfirm);
            if (m_selected == 0)
                m_paused = false;
            else
                g_pApp->ChangeScene(SCENE_PLAYRESULT);
        }
        return 0;
    }
    const float x = static_cast<float>(input->Held(VK_RIGHT)) - input->Held(VK_LEFT);
    const float y = static_cast<float>(input->Held(VK_DOWN)) - input->Held(VK_UP);
    if (Tick(g_pApp->DeltaSeconds(), x, y, input->Held(VK_SHIFT), input->Held('Z')))
        g_pApp->Resources().PlaySound(SoundId::Shot);
    return 0;
}

int ScenePlay::Render()
{
    auto& resources = g_pApp->Resources();
    resources.Draw(ImageId::Forest, 0, 0, 1.0F, 0xFFA0A0A0);
    m_bullets.Render(resources);
    for (const auto& mob : m_mobs)
        mob.Render(resources);
    m_player.Render(resources);
    resources.Draw(ImageId::Panel, 480, 0);
    Text(520, 45, "花 · 꽃빛 요정의 숲", 0xFFFFD98D);
    resources.Draw(ImageId::LeonPortrait, 520, 100, 0.3F);
    Text(640, 115, "레온 애쉬포드");
    Text(640, 160, "기본 동작 시연", 0xFFB9C9F3);
    char timer[80]{};
    std::snprintf(timer, sizeof(timer), "시연 시간  %.1f초", static_cast<double>(m_elapsedSeconds));
    Text(520, 285, timer);
    Text(520, 350, "방향키   이동");
    Text(520, 390, "Z            연속 사격");
    Text(520, 430, "Shift      저속 이동");
    Text(520, 470, "Esc         일시정지");
    Text(520, 540, "RebornBoss", 0xFFB9C9F3);
    if (m_paused)
    {
        resources.Draw(ImageId::Panel, 0, 0, 2.0F, 0xF0FFFFFF);
        Text(300, 180, "일시정지", 0xFFFFD98D, true);
        Text(320, 280, m_selected == 0 ? "> 계속" : "   계속",
             m_selected == 0 ? 0xFFFFD98D : 0xFFBCC4D4);
        Text(320, 340, m_selected == 1 ? "> 시연 종료" : "   시연 종료",
             m_selected == 1 ? 0xFFFFD98D : 0xFFBCC4D4);
        Text(300, 440, "Enter로 선택 · Esc로 계속", 0xFFBCC4D4);
    }
    return 0;
}
