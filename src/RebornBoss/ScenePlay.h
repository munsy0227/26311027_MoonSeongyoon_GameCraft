#pragma once
#include "GameScene.h"
#include "GamePlayer.h"
#include "GameMob.h"
#include "GameBossMob.h"
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
    friend void CheckGameplay();
    enum class StagePhase { Intro, Waves, BossIntro, Boss, Finished };
    enum Event : unsigned { Shot = 1, Hit = 2, EnemyDown = 4, Bomb = 8 };
    struct Effect { float x; float y; float age{}; };
    void Reset();
    bool Tick(float seconds, float directionX, float directionY, bool slow,
              bool shooting, bool bombPressed = false);
    void Step(float seconds, float directionX, float directionY, bool slow, bool shooting);
    void Collide();
    void Finish(bool cleared);
    GamePlayer m_player;
    BulletManager m_bullets;
    std::vector<GameMob> m_mobs;
    GameBossMob m_boss;
    std::vector<Effect> m_effects;
    StagePhase m_stagePhase{StagePhase::Intro};
    float m_elapsedSeconds{};
    float m_waveSeconds{};
    float m_spawnTimer{0.75F};
    float m_hitEffectSeconds{};
    float m_bombEffectSeconds{};
    int m_spawnCount{};
    int m_score{};
    int m_selected{};
    unsigned m_events{};
    bool m_paused{};
    bool m_cleared{};
};
