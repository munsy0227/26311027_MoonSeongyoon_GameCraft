#pragma once
#include "GameCharacter.h"
class BulletManager;
class GameResources;

class GameBossMob : public GameCharacter
{
public:
    static constexpr int MaxHealth = 500;
    static constexpr float Radius = 24.0F;
    GameBossMob();
    void Update(float seconds, BulletManager& bullets);
    bool AdvancePhase(BulletManager& bullets);
    bool PhaseTwo() const { return m_phaseTwo; }
    void Render(const GameResources& resources) const;

private:
    float m_animationSeconds{};
    float m_shotTimer{1.2F};
    bool m_phaseTwo{};
};
