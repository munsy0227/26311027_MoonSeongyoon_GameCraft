#pragma once
#include "GameScene.h"
#include "GamePlayer.h"
#include "GameMob.h"
#include "BulletManager.h"
#include <vector>

class ScenePlay : public GameScene
{
public:
    int Init() override;
    int Destroy() override;
    int Update() override;
    int Render() override;

private:
    friend void CheckWeek2Gameplay();
    bool Tick(float seconds, float directionX, float directionY, bool slow, bool shooting);
    GamePlayer m_player;
    BulletManager m_bullets;
    std::vector<GameMob> m_mobs;
    float m_elapsedSeconds{};
    float m_spawnTimer{0.75F};
    int m_spawnCount{};
    int m_selected{};
    bool m_paused{};
};
