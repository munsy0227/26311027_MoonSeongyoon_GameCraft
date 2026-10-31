#include "SceneBegin.h"
#include "GameInput.h"
#include "App.h"
#include <windows.h>

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
    MoveSelection(m_selected, 3);
    if (input->Pressed(VK_RETURN))
    {
        g_pApp->Resources().PlaySound(SoundId::MenuConfirm);
        if (m_selected == 0)
            g_pApp->ChangeScene(SCENE_LOBBY);
        else if (m_selected == 1)
            m_controls = true;
        else
            PostQuitMessage(0);
    }
    return 0;
}

int SceneBegin::Render()
{
    auto& resources = g_pApp->Resources();
    resources.Draw(ImageId::Title, 0, 0);
    resources.Rect(0, 0, 960, 600, 0xC80E111A);
    if (m_controls)
    {
        resources.Surface(152, 56, 656, 488);
        Text(184, 80, "조작법", UIColor::Text, FontStyle::Heading);
        Text(184, 120, "작은 중심점으로 탄막 사이를 피하세요.", UIColor::Muted, FontStyle::Caption);
        constexpr const char* keys[] = {"방향키", "Z", "Shift", "X", "Enter", "Esc"};
        constexpr const char* descriptions[] = {
            "이동 / 메뉴 선택", "누르는 동안 연속 사격", "저속 이동 / 피격 중심점 표시",
            "폭탄 사용 / 적탄 제거", "선택 / 대사 진행", "일시정지 / 이전 화면"
        };
        for (int i = 0; i < 6; ++i)
        {
            Text(184, 172 + i * 42, keys[i], UIColor::Accent);
            Text(328, 172 + i * 42, descriptions[i]);
        }
        Text(184, 444, "생명 3개 · 폭탄 3개로 시작합니다.", UIColor::Muted, FontStyle::Caption);
        Text(184, 480, "Enter 또는 Esc로 돌아가기", UIColor::Muted, FontStyle::Caption);
    }
    else
    {
        Text(64, 48, "RebornBoss", UIColor::Accent, FontStyle::Caption);
        Text(64, 104, "이세계에 전생했더니", UIColor::Text, FontStyle::Title);
        Text(64, 156, "요정과 요괴들의", UIColor::Text, FontStyle::Title);
        Text(64, 208, "최종 보스가 되었습니다", UIColor::Text, FontStyle::Title);
        Text(68, 276, "수호자들의 탄막을 넘어, 최종 보스가 되어 보세요.", UIColor::Muted, FontStyle::Caption);
        resources.Draw(ImageId::LeonPortrait, 672, 138, 0.55F);
        MenuItem(64, 340, "게임 시작", m_selected == 0);
        MenuItem(64, 404, "조작법", m_selected == 1);
        MenuItem(64, 468, "종료", m_selected == 2);
        Text(64, 552, "방향키로 선택 · Enter로 결정", UIColor::Muted, FontStyle::Caption);
    }
    return 0;
}
