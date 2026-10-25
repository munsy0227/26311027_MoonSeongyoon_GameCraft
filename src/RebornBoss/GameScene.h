#pragma once
#include <glc2d.h>

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
    void Text(int x, int y, const char* text, DWORD color = 0xFFF4EBD7,
              bool title = false) const;
    bool MoveSelection(int& selected, int count) const;
};
