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
    if (input->CancelPressed())
        g_pApp->ChangeScene(SCENE_LOBBY);
    else if (input->ConfirmPressed())
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
    MenuBackground();
    resources.Draw(ImageId::LeonArt, 550, 35, 0.56F);
    Text(64, 64, "STAGE 01 · 꽃빛 요정의 숲", UIColor::Muted, FontStyle::Caption);
    Text(64, 120, result.cleared ? "스테이지 클리어" : "다시 도전해 보세요",
         result.cleared ? UIColor::Accent : UIColor::Text, FontStyle::Title);
    resources.Rect(64, 188, 96, 1, 0x80A99CFF);
    Text(64, 224, "획득 점수", UIColor::Muted, FontStyle::Caption);
    char score[64]{};
    std::snprintf(score, sizeof(score), "%d", result.score);
    Text(64, 256, score, UIColor::Text, FontStyle::Title);
    char timer[80]{};
    std::snprintf(timer, sizeof(timer), "전투 시간  %.1f초", static_cast<double>(result.elapsedSeconds));
    Text(64, 326, timer, UIColor::Muted, FontStyle::Caption);
    MenuItem(90, 390, "재도전", m_selected == 0);
    MenuItem(90, 452, "스테이지 선택", m_selected == 1);
    Text(64, 552, GetGameInput()->IsNewClassic() ? "Z / J / Enter 결정 · X / K / Esc 스테이지 선택"
                                               : "Z로 결정 · X / Esc로 스테이지 선택",
         UIColor::Muted, FontStyle::Caption);
    return 0;
}
