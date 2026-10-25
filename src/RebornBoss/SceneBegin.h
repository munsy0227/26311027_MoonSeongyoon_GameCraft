#pragma once
#include "GameScene.h"

class SceneBegin : public GameScene
{
public:
    int Init() override;
    int Update() override;
    int Render() override;

private:
    int m_selected{};
    bool m_controls{};
};
