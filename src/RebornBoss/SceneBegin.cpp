#include "SceneBegin.h"
#include "GameInput.h"
#include "App.h"

int SceneBegin::Init()
{
    m_selected = 0;
    m_controls = false;
    g_pApp->Resources().PlayBgm(SoundId::Menu);
    return 0;
}

int SceneBegin::Update()
{
    const auto* input = GetGameInput();
    if (m_controls)
    {
        if (input->Pressed(VK_ESCAPE) || input->Pressed(VK_RETURN))
            m_controls = false;
        return 0;
    }
    MoveSelection(m_selected, 2);
    if (input->Pressed(VK_RETURN))
    {
        g_pApp->Resources().PlaySound(SoundId::MenuConfirm);
        if (m_selected == 0)
            g_pApp->ChangeScene(SCENE_LOBBY);
        else
            m_controls = true;
    }
    return 0;
}

int SceneBegin::Render()
{
    auto& resources = g_pApp->Resources();
    resources.Draw(ImageId::Title, 0, 0, 1.0F, 0xFFB0B0B0);
    if (m_controls)
    {
        resources.Draw(ImageId::Panel, 0, 0, 2.0F, 0xF0FFFFFF);
        Text(100, 70, "조작법", 0xFFFFD98D, true);
        Text(100, 150, "방향키    이동 / 메뉴 선택");
        Text(100, 200, "Z             누르고 있는 동안 연속 사격");
        Text(100, 250, "Shift       저속 이동 / 중심점 표시");
        Text(100, 300, "Esc          일시정지 / 이전 화면");
        Text(100, 350, "Enter       메뉴 선택");
        Text(100, 470, "Enter 또는 Esc로 돌아가기", 0xFFB8CFD6);
    }
    else
    {
        Text(80, 90, "이세계에 전생했더니", 0xFFFFE8B0, true);
        Text(80, 145, "요정과 요괴들의 최종 보스가 되었습니다", 0xFFFFFFFF, true);
        Text(84, 215, "RebornBoss", 0xFFBFC9EF);
        Text(100, 325, m_selected == 0 ? "> 게임 시작" : "   게임 시작",
             m_selected == 0 ? 0xFFFFD98D : 0xFFB8CFD6);
        Text(100, 380, m_selected == 1 ? "> 조작법" : "   조작법",
             m_selected == 1 ? 0xFFFFD98D : 0xFFB8CFD6);
        Text(80, 535, "방향키로 선택 · Enter로 결정", 0xFFCBD1E7);
    }
    return 0;
}
