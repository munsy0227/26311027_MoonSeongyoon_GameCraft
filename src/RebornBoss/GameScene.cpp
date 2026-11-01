#include "GameScene.h"
#include "GameInput.h"
#include "App.h"

void GameScene::Text(int x, int y, const char* text, DWORD color, FontStyle style) const
{
    const int font = g_pApp->Resources().Font(style);
    g2_FontDrawText(font, VEC2i{x, y}, color, "%s", text);
}

void GameScene::MenuBackground() const
{
    const auto& resources = g_pApp->Resources();
    resources.Draw(ImageId::Title, 0, 0);
    resources.Rect(0, 0, 960, 600, 0x50080B13);
    resources.Rect(0, 0, 480, 600, 0xCE080B13);
    for (int i = 0; i < 32; ++i)
    {
        const DWORD color = (static_cast<DWORD>((32 - i) * 6) << 24) | 0x00080B13;
        resources.Rect(480.0F + i * 6.0F, 0, 6, 600, color);
    }
}

void GameScene::MenuItem(int x, int y, const char* label, bool selected, float width) const
{
    const auto& resources = g_pApp->Resources();
    if (selected)
    {
        resources.Rect(static_cast<float>(x), y + 20.0F, 14, 2, UIColor::Accent);
        resources.Rect(x + 30.0F, y + 40.0F, width * 0.16F, 1, 0x80A99CFF);
    }
    Text(x + 30, y + 7, label, selected ? UIColor::Accent : UIColor::Muted);
}

bool GameScene::MoveSelection(int& selected, int count) const
{
    const auto* input = GetGameInput();
    const int direction = input->MenuDirection();
    if (direction == 0)
        return false;
    selected = (selected + direction + count) % count;
    g_pApp->Resources().PlaySound(SoundId::MenuMove);
    return true;
}
