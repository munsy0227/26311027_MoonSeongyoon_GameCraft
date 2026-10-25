#pragma once
#include "GameScene.h"

class ScenePlayResult : public GameScene
{
public:
    int Init() override;
    int Update() override;
    int Render() override;

private:
    int m_selected{};
};
