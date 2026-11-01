#pragma once
#include "GameCharacter.h"
class BulletManager;
class GameResources;

class GameBossMob : public GameCharacter
{
public:
    static constexpr int MaxHealth = 500;
    static constexpr float Radius = 24.0F;
    static constexpr float RenderScale = 1.615F;
    static constexpr float RenderSize = 64.0F * RenderScale;
    static constexpr float BaseY = 144.0F;
    GameBossMob();
    void Update(float seconds, BulletManager& bullets, float playerX, float playerY);
    bool AdvancePhase(BulletManager& bullets);
    bool PhaseTwo() const { return m_phaseTwo; }
    void Render(const GameResources& resources) const;

private:
    double m_animationSeconds{};
    float m_shotTimer{0.85F};
    float m_ringAngle{};
    bool m_aimNextRing{};
    bool m_phaseTwo{};
};
