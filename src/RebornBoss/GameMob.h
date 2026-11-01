#pragma once
#include "GameCharacter.h"
class BulletManager;
class GameResources;

enum class MobType { Patrol, Familiar, Guardian };

class GameMob : public GameCharacter
{
public:
    static constexpr float Radius = 14.0F;
    explicit GameMob(float x = 240.0F, MobType type = MobType::Patrol);
    MobType Type() const { return m_type; }
    void Update(float seconds);
    void Update(float seconds, BulletManager& bullets, float playerX, float playerY);
    void Render(const GameResources& resources) const;
    bool Outside() const { return m_y > 616.0F; }

private:
    MobType m_type;
    float m_spawnX;
    double m_animationSeconds{};
    float m_shotTimer{};
};
