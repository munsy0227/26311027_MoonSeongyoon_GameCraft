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
    MenuBackground();
    constexpr ImageId characters[] = {
        ImageId::LillianArt, ImageId::VivianArt, ImageId::EleanorArt
    };
    resources.Draw(characters[m_selected], 550, 35, 0.56F,
                   m_selected == 0 ? 0xFFFFFFFF : 0xFFB0B0B0);
    Text(64, 48, "스테이지 선택", UIColor::Text, FontStyle::Title);
    Text(64, 112, "수호자의 영역으로", UIColor::Muted, FontStyle::Caption);
    resources.Rect(64, 152, 96, 1, 0x80A99CFF);
    constexpr const char* numbers[] = {"01", "02", "03"};
    constexpr const char* names[] = {"꽃빛 요정의 숲", "안개 요괴의 호수", "월광의 결계 신전"};
    constexpr const char* bosses[] = {"릴리안 블룸", "비비안 미스트우드", "엘레노어 나이트베일"};
    constexpr const char* requirements[] = {"Enter로 시작", "1스테이지 클리어 후 해금 예정", "2스테이지 클리어 후 해금 예정"};
    for (int i = 0; i < 3; ++i)
    {
        const int y = 182 + i * 108;
        const bool selected = i == m_selected;
        if (selected)
            resources.Rect(72, y + 17.0F, 14, 2, UIColor::Accent);
        Text(100, y + 6, numbers[i], selected ? UIColor::Accent : UIColor::Muted, FontStyle::Caption);
        Text(148, y, names[i], selected ? UIColor::Text : UIColor::Muted, FontStyle::Heading);
        Text(148, y + 38, bosses[i], UIColor::Muted, FontStyle::Caption);
        Text(148, y + 66, i == 0 ? "플레이 가능" : "준비 중",
             selected ? UIColor::Accent : UIColor::Muted, FontStyle::Caption);
        if (i != 0)
            resources.Draw(ImageId::Lock, 400, y + 64.0F, 0.75F);
        if (i != 2)
            resources.Rect(148, y + 94.0F, 280, 1, 0x5039425A);
    }
    Text(64, 516, m_lockedNotice ? "아직 준비 중인 스테이지입니다." : requirements[m_selected],
         m_lockedNotice ? UIColor::Accent : UIColor::Muted, FontStyle::Caption);
    Text(64, 556, "방향키로 선택 · Enter로 시작 · Esc로 돌아가기", UIColor::Muted, FontStyle::Caption);
    return 0;
}
