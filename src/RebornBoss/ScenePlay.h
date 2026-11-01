#pragma once
#include "GameScene.h"
#include "GamePlayer.h"
#include "GameMob.h"
#include "GameBossMob.h"
#include "BulletManager.h"
#include <vector>
#include <random>

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
    enum Event : unsigned { Shot = 1, Hit = 2, EnemyDown = 4, Bomb = 8, ItemCollected = 16 };
    enum class PickupType { Score, Power, Health, Bomb };
    struct Pickup { float x; float y; PickupType type; float age{}; bool attracted{}; };
    struct ScorePopup { float x; float y; float age{}; };
    struct Effect { float x; float y; float age{}; };
    void Reset();
    bool Tick(float seconds, float directionX, float directionY, bool slow,
              bool shooting, bool bombPressed = false);
    void Step(float seconds, float directionX, float directionY, bool slow, bool shooting);
    void Collide();
    void DropLoot(float x, float y, int roll);
    void UpdatePickups(float seconds);
    void Finish(bool cleared);
    int DialogueCount() const;
    bool UpdateDialogue(float seconds, bool advanceHeld, bool advancePressed, bool fastHeld);
    GamePlayer m_player;
    BulletManager m_bullets;
    std::vector<GameMob> m_mobs;
    GameBossMob m_boss;
    std::vector<Effect> m_effects;
    std::vector<Pickup> m_pickups;
    std::vector<ScorePopup> m_scorePopups;
    std::mt19937 m_dropRandom{0x52424F53U};
    StagePhase m_stagePhase{StagePhase::Intro};
    float m_elapsedSeconds{};
    float m_waveSeconds{};
    float m_spawnTimer{0.75F};
    float m_hitEffectSeconds{};
    float m_bombEffectSeconds{};
    float m_fastDialogueSeconds{};
    int m_spawnCount{};
    int m_score{};
    int m_selected{};
    int m_dialogueIndex{};
    unsigned m_events{};
    bool m_paused{};
    bool m_cleared{};
    bool m_dialogueReady{};
};
