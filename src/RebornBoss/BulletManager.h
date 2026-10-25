#pragma once
#include <vector>
#include <cstddef>
#include "GameBullet.h"
class GameResources;

class BulletManager
{
public:
    bool Update(float seconds, bool shooting, float playerX, float playerY);
    void Render(const GameResources& resources) const;
    void Clear();
    std::size_t Count() const { return m_bullets.size(); }

private:
    std::vector<GameBullet> m_bullets;
    float m_shotTimer{};
};
