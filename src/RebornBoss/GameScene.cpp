#include "GameScene.h"
#include "GameInput.h"
#include "App.h"

void GameScene::Text(int x, int y, const char* text, DWORD color, bool title) const
{
    const auto& resources = g_pApp->Resources();
    const int font = title ? resources.TitleFont() : resources.Font();
    g2_FontDrawText(font, VEC2i{x + 2, y + 2}, 0xE0000000, "%s", text);
    g2_FontDrawText(font, VEC2i{x, y}, color, "%s", text);
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
