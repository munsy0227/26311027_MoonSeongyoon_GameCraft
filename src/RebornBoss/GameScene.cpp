#include "GameScene.h"
#include "GameInput.h"
#include "App.h"

void GameScene::Text(int x, int y, const char* text, DWORD color, FontStyle style) const
{
    const int font = g_pApp->Resources().Font(style);
    g2_FontDrawText(font, VEC2i{x, y}, color, "%s", text);
}

void GameScene::MenuItem(int x, int y, const char* label, bool selected, float width) const
{
    const auto& resources = g_pApp->Resources();
    resources.Surface(static_cast<float>(x), static_cast<float>(y), width, 48.0F,
                      selected ? UIColor::Accent : UIColor::Border);
    resources.Surface(x + 1.0F, y + 1.0F, width - 2.0F, 46.0F,
                      selected ? 0xFF302B47 : UIColor::Surface);
    Text(x + 24, y + 11, label, selected ? UIColor::Text : UIColor::Muted);
}

bool GameScene::MoveSelection(int& selected, int count) const
{
    const auto* input = GetGameInput();
    const int direction = (input->Pressed(VK_DOWN) || input->Pressed(VK_RIGHT) ? 1 : 0)
                        - (input->Pressed(VK_UP) || input->Pressed(VK_LEFT) ? 1 : 0);
    if (direction == 0)
        return false;
    selected = (selected + direction + count) % count;
    g_pApp->Resources().PlaySound(SoundId::MenuMove);
    return true;
}
