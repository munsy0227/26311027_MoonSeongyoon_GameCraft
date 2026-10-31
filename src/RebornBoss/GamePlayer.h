#pragma once
#include "GameCharacter.h"
class GameResources;

class GamePlayer : public GameCharacter
{
public:
    static constexpr float Radius = 4.0F;
    GamePlayer();
    void Update(float seconds, float directionX, float directionY, bool slow, bool shooting);
    void Render(const GameResources& resources) const;
    int Damage(int damage) override;
    bool UseBomb();
    int Bombs() const { return m_bombs; }
    float InvulnerabilitySeconds() const { return m_invulnerabilitySeconds; }

private:
    float m_animationSeconds{};
    float m_invulnerabilitySeconds{};
    int m_bombs{3};
    int m_row{};
    bool m_slow{};
};
