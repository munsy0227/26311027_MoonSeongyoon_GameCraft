#include "ScenePlayResult.h"
#include "GameInput.h"
#include "App.h"

int ScenePlayResult::Init()
{
    m_selected = 0;
    g_pApp->Resources().PlayBgm(SoundId::Menu);
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
    resources.Draw(ImageId::Title, 0, 0, 1.0F, 0xFF707080);
    Text(90, 80, "시연 종료", 0xFFFFD98D, true);
    Text(90, 160, "꽃빛 요정의 숲");
    Text(90, 210, "기본 동작 시연을 종료했습니다.");
    Text(110, 330, m_selected == 0 ? "> 다시 시작" : "   다시 시작",
         m_selected == 0 ? 0xFFFFD98D : 0xFFBCC4D4);
    Text(110, 390, m_selected == 1 ? "> 스테이지 선택" : "   스테이지 선택",
         m_selected == 1 ? 0xFFFFD98D : 0xFFBCC4D4);
    resources.Draw(ImageId::LeonPortrait, 680, 210, 0.5F);
    Text(90, 535, "방향키로 선택 · Enter로 결정", 0xFFCBD1E7);
    return 0;
}
