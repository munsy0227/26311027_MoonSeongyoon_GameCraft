#include "SceneBegin.h"
#include "GameInput.h"
#include "App.h"
#include <windows.h>
#include <cmath>

int SceneBegin::Init()
{
    m_selected = 0;
    m_controls = false;
    m_backgroundSeconds = 0.0F;
    g_pApp->Resources().PlayBgm(SoundId::Menu);
    return 0;
}

int SceneBegin::Update()
{
    m_backgroundSeconds = std::fmod(m_backgroundSeconds + g_pApp->DeltaSeconds(), 20.0F);
    auto* input = GetGameInput();
    if (m_controls)
    {
        const int direction = (input->Pressed(VK_RIGHT) || input->Pressed(VK_NUMPAD6)
                               || (input->IsNewClassic() && input->Pressed('D')) ? 1 : 0)
                            - (input->Pressed(VK_LEFT) || input->Pressed(VK_NUMPAD4)
                               || (input->IsNewClassic() && input->Pressed('A')) ? 1 : 0);
        if (direction != 0)
        {
            input->ToggleControlPreset();
            g_pApp->Resources().PlaySound(SoundId::MenuMove);
            return 0;
        }
        if (input->CancelPressed() || input->ConfirmPressed())
            m_controls = false;
        return 0;
    }
    MoveSelection(m_selected, 3);
    if (input->ConfirmPressed())
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
    // Overscan keeps every viewport edge covered during the slow drift.
    const float angle = m_backgroundSeconds * (6.283185307F / 20.0F);
    const float x = -12.0F + 8.0F * std::sin(angle);
    const float y = -7.5F + 4.0F * std::sin(angle + 1.570796327F);
    resources.Draw(ImageId::TitleAtmosphere, x, y, 1.025F);
    resources.Rect(0, 0, 480, 600, 0x78080B13);
    for (int i = 0; i < 24; ++i)
    {
        const DWORD color = (static_cast<DWORD>((24 - i) * 5) << 24) | 0x00080B13;
        resources.Rect(480.0F + i * 8.0F, 0, 8, 600, color);
    }
    if (m_controls)
    {
        resources.Rect(0, 0, 960, 600, 0x98080B13);
        Text(64, 48, "조작법", UIColor::Text, FontStyle::Title);
        const bool remake = GetGameInput()->IsNewClassic();
        constexpr const char* presets[] = {"기본 동방", "홍마향 리메이크"};
        constexpr float tabX[] = {64, 264};
        constexpr float tabWidth[] = {184, 244};
        for (int i = 0; i < 2; ++i)
        {
            const bool selected = remake == (i == 1);
            resources.Rect(tabX[i], 110, tabWidth[i], 36, selected ? UIColor::Accent : UIColor::Border);
            resources.Rect(tabX[i] + 1, 111, tabWidth[i] - 2, 34, selected ? 0xFF302B47 : 0xFF121722);
            Text(static_cast<int>(tabX[i]) + 18, 117, presets[i], selected ? UIColor::Text : UIColor::Muted);
        }
        Text(64, 156, "좌우로 변경 · 선택은 이번 실행 동안 유지", UIColor::Muted, FontStyle::Caption);
        resources.Rect(64, 188, 832, 1, UIColor::Border);
        constexpr const char* classicKeys[] = {"방향키 / Num", "Z", "X", "Shift", "Esc"};
        constexpr const char* remakeKeys[] = {"방향키 / WASD", "Z / J", "X / K", "LShift·Space·L", "Esc"};
        constexpr const char* descriptions[] = {
            "이동 / 메뉴 선택", "누르는 동안 사격 / 결정 / 대사", "폭탄 사용 / 메뉴 취소",
            "저속 이동 / 피격 중심점 표시", "일시정지 / 이전 화면"
        };
        for (int i = 0; i < 5; ++i)
        {
            Text(96, 212 + i * 48, remake ? remakeKeys[i] : classicKeys[i], UIColor::Accent);
            Text(280, 212 + i * 48, descriptions[i]);
            resources.Rect(96, 248.0F + i * 48, 768, 1, 0x5039425A);
        }
        Text(64, 454, remake ? "Enter: 결정 · 왼쪽 Ctrl / ;: 대사 빨리 넘기기" : "숫자패드로도 이동할 수 있습니다.",
             UIColor::Muted, FontStyle::Caption);
        Text(64, 484, remake ? "일시정지 중 R 재도전 · Q 타이틀" : "생명 3개 · 폭탄 3개로 시작합니다.",
             UIColor::Muted, FontStyle::Caption);
        if (remake)
            Text(64, 514, "기본 동방 키와 숫자패드도 함께 사용할 수 있습니다.", UIColor::Muted, FontStyle::Caption);
        Text(64, 556, remake ? "Z / J / Enter 결정 · X / K / Esc 돌아가기" : "Z로 결정 · X / Esc로 돌아가기",
             UIColor::Muted, FontStyle::Caption);
    }
    else
    {
        Text(64, 44, "RebornBoss", UIColor::Accent, FontStyle::Caption);
        Text(64, 104, "이세계에 전생했더니", UIColor::Text, FontStyle::Title);
        Text(64, 156, "요정과 요괴들의", UIColor::Text, FontStyle::Title);
        Text(64, 208, "최종 보스가 되었습니다", UIColor::Text, FontStyle::Title);
        resources.Rect(64, 280, 96, 1, 0x80A99CFF);
        Text(64, 302, "수호자들의 탄막을 넘어", UIColor::Muted, FontStyle::Caption);
        MenuItem(90, 350, "게임 시작", m_selected == 0);
        MenuItem(90, 404, "조작법", m_selected == 1);
        MenuItem(90, 458, "종료", m_selected == 2);
        Text(64, 552, GetGameInput()->IsNewClassic() ? "방향키 / WASD / Num 선택 · Z / J / Enter 결정"
                                                   : "방향키 / Num으로 선택 · Z로 결정",
             UIColor::Muted, FontStyle::Caption);
    }
    return 0;
}
