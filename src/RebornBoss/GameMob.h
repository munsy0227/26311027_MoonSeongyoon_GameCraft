#pragma once
#include "GameCharacter.h"
class BulletManager;
class GameResources;

enum class MobType { Patrol, Familiar, Guardian };

class GameMob : public GameCharacter
{
public:
    static constexpr float Radius = 14.0F;
    static constexpr float RenderScale = 1.25F;
    explicit GameMob(float x = 240.0F, MobType type = MobType::Patrol);
    MobType Type() const { return m_type; }
    void Update(float seconds);
    void Update(float seconds, BulletManager& bullets, float playerX, float playerY);
    void Render(const GameResources& resources) const;
    int CellSize() const { return m_type == MobType::Guardian ? 40 : 32; }
    float RenderSize() const { return CellSize() * RenderScale; }
    bool Outside() const { return m_y > 600.0F + RenderSize() * 0.5F; }

private:
    MobType m_type;
    float m_spawnX;
    double m_animationSeconds{};
    float m_shotTimer{};
};
