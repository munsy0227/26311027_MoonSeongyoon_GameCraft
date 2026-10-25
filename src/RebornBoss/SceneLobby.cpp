#include "SceneLobby.h"
#include "GameInput.h"
#include "App.h"

int SceneLobby::Init()
{
    m_selected = 0;
    m_lockedNotice = false;
    g_pApp->Resources().PlayBgm(SoundId::Menu);
    return 0;
}

int SceneLobby::Update()
{
    const auto* input = GetGameInput();
    if (MoveSelection(m_selected, 3))
        m_lockedNotice = false;
    if (input->Pressed(VK_ESCAPE))
        g_pApp->ChangeScene(SCENE_BEGIN);
    else if (input->Pressed(VK_RETURN))
    {
        if (m_selected == 0)
        {
            g_pApp->Resources().PlaySound(SoundId::MenuConfirm);
            g_pApp->ChangeScene(SCENE_PLAY);
        }
        else
            m_lockedNotice = true;
    }
    return 0;
}

int SceneLobby::Render()
{
    auto& resources = g_pApp->Resources();
    resources.Draw(ImageId::Title, 0, 0, 1.0F, 0xFF707080);
    Text(40, 45, "스테이지 선택", 0xFFFFD98D, true);
    constexpr ImageId portraits[] = {
        ImageId::LillianPortrait, ImageId::VivianPortrait, ImageId::EleanorPortrait
    };
    constexpr const char* names[] = {"꽃빛 요정의 숲", "안개 요괴의 호수", "월광의 결계 신전"};
    for (int i = 0; i < 3; ++i)
    {
        const float x = 24.0F + i * 312.0F;
        resources.Draw(ImageId::Panel, x, 130, 0.6F,
                       i == m_selected ? 0xFFFFFFFF : 0xFFA0A0A0);
        resources.Draw(portraits[i], x + 72.0F, 170, 0.45F,
                       i == 0 ? 0xFFFFFFFF : 0xFF777777);
        Text(static_cast<int>(x) + 26, 405, names[i],
             i == m_selected ? 0xFFFFD98D : 0xFFBCC4D4);
        Text(static_cast<int>(x) + 26, 450, i == 0 ? "1 STAGE  ·  시작" : "잠김");
        if (i != 0)
            resources.Draw(ImageId::Lock, x + 232.0F, 450);
    }
    Text(40, 535, m_lockedNotice ? "아직 열리지 않은 스테이지입니다."
                               : "방향키로 선택 · Enter로 시작 · Esc로 돌아가기",
         m_lockedNotice ? 0xFFFFC595 : 0xFFCBD1E7);
    return 0;
}
