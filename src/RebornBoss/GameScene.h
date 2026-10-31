#pragma once
#include "GameResources.h"

enum GAME_PHASE : int { SCENE_BEGIN = 0, SCENE_LOBBY, SCENE_PLAY, SCENE_PLAYRESULT };

class GameScene
{
public:
    virtual ~GameScene() = default;
    virtual int Init() { return 0; }
    virtual int Destroy() { return 0; }
    virtual int Update() { return 0; }
    virtual int Render() { return 0; }

protected:
    void Text(int x, int y, const char* text, DWORD color = UIColor::Text,
              FontStyle style = FontStyle::Body) const;
    void MenuBackground() const;
    void MenuItem(int x, int y, const char* label, bool selected, float width = 344.0F) const;
    bool MoveSelection(int& selected, int count) const;
};
