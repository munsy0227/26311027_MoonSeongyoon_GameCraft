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
    resources.Draw(ImageId::Title, 0, 0);
    resources.Rect(0, 0, 960, 600, 0xD90E111A);
    Text(32, 40, "스테이지 선택", UIColor::Text, FontStyle::Title);
    Text(32, 96, "첫 번째 숲에서 새로운 이야기를 시작하세요.", UIColor::Muted, FontStyle::Caption);
    constexpr ImageId portraits[] = {
        ImageId::LillianPortrait, ImageId::VivianPortrait, ImageId::EleanorPortrait
    };
    constexpr const char* names[] = {"꽃빛 요정의 숲", "안개 요괴의 호수", "월광의 결계 신전"};
    constexpr const char* bosses[] = {"릴리안 블룸", "비비안 미스트우드", "엘레노어 나이트베일"};
    constexpr const char* requirements[] = {"Enter로 시작", "1스테이지 클리어 후 해금 예정", "2스테이지 클리어 후 해금 예정"};
    for (int i = 0; i < 3; ++i)
    {
        const float x = 24.0F + i * 312.0F;
        const bool selected = i == m_selected;
        resources.Surface(x, 144, 288, 360, selected ? UIColor::Accent : UIColor::Border);
        resources.Surface(x + 1, 145, 286, 358, selected ? 0xFF25243A : UIColor::Surface);
        resources.Draw(portraits[i], x + 83, 172, 0.38F, i == 0 ? 0xFFFFFFFF : 0xFF999999);
        Text(static_cast<int>(x) + 24, 376, names[i]);
        Text(static_cast<int>(x) + 24, 412, bosses[i], UIColor::Muted, FontStyle::Caption);
        Text(static_cast<int>(x) + 24, 460, i == 0 ? "01  ·  플레이 가능" : i == 1 ? "02  ·  준비 중" : "03  ·  준비 중",
             i == 0 ? UIColor::Accent : UIColor::Muted, FontStyle::Caption);
        if (i != 0)
            resources.Draw(ImageId::Lock, x + 240, 458);
    }
    Text(32, 528, m_lockedNotice ? "아직 준비 중인 스테이지입니다." : requirements[m_selected],
         m_lockedNotice ? UIColor::Accent : UIColor::Muted, FontStyle::Caption);
    Text(32, 564, "방향키로 선택 · Enter로 시작 · Esc로 돌아가기", UIColor::Muted, FontStyle::Caption);
    return 0;
}
