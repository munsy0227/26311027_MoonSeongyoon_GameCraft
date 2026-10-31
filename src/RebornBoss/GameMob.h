#pragma once
#include "GameCharacter.h"
class BulletManager;
class GameResources;

class GameMob : public GameCharacter
{
public:
    static constexpr float Radius = 14.0F;
    explicit GameMob(float x = 240.0F);
    void Update(float seconds);
    void Update(float seconds, BulletManager& bullets);
    void Render(const GameResources& resources) const;
    bool Outside() const { return m_y > 616.0F; }

private:
    float m_animationSeconds{};
    float m_shotTimer{1.5F};
};
