#pragma once
#include <vector>
#include <cstddef>
#include "GameBullet.h"
class GameResources;

class BulletManager
{
public:
    bool Update(float seconds, bool shooting, float playerX, float playerY, int power = 0);
    void Render(const GameResources& resources) const;
    void Add(GameBullet bullet);
    void AddAimedFan(float x, float y, float targetX, float targetY,
                     int count, float stepDegrees, float speed);
    std::vector<GameBullet>& Bullets() { return m_bullets; }
    const std::vector<GameBullet>& Bullets() const { return m_bullets; }
    void ClearEnemy();
    void RemoveInactive();
    void Clear();
    std::size_t Count() const { return m_bullets.size(); }

private:
    std::vector<GameBullet> m_bullets;
    float m_shotTimer{};
};
