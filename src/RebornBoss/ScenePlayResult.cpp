#include "ScenePlayResult.h"
#include "GameInput.h"
#include "App.h"
#include <cstdio>

int ScenePlayResult::Init()
{
    m_selected = 0;
    auto& resources = g_pApp->Resources();
    resources.PlayBgm(SoundId::Menu);
    resources.PlaySound(g_pApp->Result().cleared ? SoundId::Clear : SoundId::Fail);
    return 0;
}

int ScenePlayResult::Update()
{
    const auto* input = GetGameInput();
    MoveSelection(m_selected, 2);
    if (input->Pressed(VK_ESCAPE))
        g_pApp->ChangeScene(SCENE_LOBBY);
    else if (input->Pressed(VK_RETURN))
    {
        g_pApp->Resources().PlaySound(SoundId::MenuConfirm);
        g_pApp->ChangeScene(m_selected == 0 ? SCENE_PLAY : SCENE_LOBBY);
    }
    return 0;
}

int ScenePlayResult::Render()
{
    auto& resources = g_pApp->Resources();
    const auto& result = g_pApp->Result();
    resources.Draw(ImageId::Title, 0, 0);
    resources.Rect(0, 0, 960, 600, 0xD90E111A);
    resources.Surface(72, 48, 816, 504);
    Text(104, 80, "꽃빛 요정의 숲", UIColor::Muted, FontStyle::Caption);
    Text(104, 128, result.cleared ? "스테이지 클리어" : "다시 도전해 보세요",
         result.cleared ? UIColor::Accent : UIColor::Text, FontStyle::Title);
    Text(104, 208, "획득 점수", UIColor::Muted, FontStyle::Caption);
    char score[64]{};
    std::snprintf(score, sizeof(score), "%d", result.score);
    Text(104, 240, score, UIColor::Text, FontStyle::Title);
    char timer[80]{};
    std::snprintf(timer, sizeof(timer), "전투 시간  %.1f초", static_cast<double>(result.elapsedSeconds));
    Text(104, 304, timer, UIColor::Muted, FontStyle::Caption);
    resources.Draw(ImageId::LeonPortrait, 624, 196, 0.46F);
    MenuItem(104, 376, "재도전", m_selected == 0);
    MenuItem(104, 440, "스테이지 선택", m_selected == 1);
    Text(104, 512, "방향키로 선택 · Enter로 결정 · Esc로 스테이지 선택", UIColor::Muted, FontStyle::Caption);
    return 0;
}
