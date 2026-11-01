#pragma once
#include "GameCharacter.h"
class GameResources;

class GamePlayer : public GameCharacter
{
public:
    static constexpr float Radius = 4.0F;
    static constexpr float RenderScale = 2.0F;
    static constexpr float RenderWidth = 32.0F * RenderScale;
    static constexpr float RenderHeight = 48.0F * RenderScale;
    static constexpr float ShotOffset = RenderHeight * 0.5F + 8.0F;
    static constexpr int MaxHealth = 5;
    static constexpr int MaxBombs = 5;
    static constexpr int MaxPower = 4;
    static constexpr int PowerPerLevel = 10;
    static constexpr int MaxPowerPoints = MaxPower * PowerPerLevel;
    GamePlayer();
    void Update(float seconds, float directionX, float directionY, bool slow, bool shooting);
    void Render(const GameResources& resources) const;
    int Damage(int damage) override;
    bool UseBomb();
    bool Heal();
    bool AddBomb();
    bool AddPower();
    int Power() const { return m_powerPoints / PowerPerLevel; }
    int PowerPoints() const { return m_powerPoints; }
    int Bombs() const { return m_bombs; }
    float InvulnerabilitySeconds() const { return m_invulnerabilitySeconds; }

private:
    float m_animationSeconds{};
    float m_velocityX{};
    float m_velocityY{};
    float m_invulnerabilitySeconds{};
    int m_bombs{3};
    int m_powerPoints{};
    int m_row{};
    bool m_slow{};
};
