#include "BulletManager.h"
#include "GameResources.h"
#include <algorithm>

bool BulletManager::Update(float seconds, bool shooting, float playerX, float playerY)
{
    for (auto& bullet : m_bullets)
        bullet.Update(seconds);
    std::erase_if(m_bullets, [](const GameBullet& bullet) { return bullet.Outside(); });
    m_shotTimer -= seconds;
    if (!shooting)
    {
        m_shotTimer = std::max(0.0F, m_shotTimer);
        return false;
    }
    bool fired = false;
    while (m_shotTimer <= 0.000001F)
    {
        m_bullets.push_back({playerX, playerY - 28.0F});
        m_shotTimer += 0.12F;
        fired = true;
    }
    return fired;
}

void BulletManager::Render(const GameResources& resources) const
{
    for (const auto& bullet : m_bullets)
        resources.Draw(ImageId::PlayerShot, bullet.x - 6.0F, bullet.y - 12.0F);
}

void BulletManager::Clear()
{
    m_bullets.clear();
    m_shotTimer = 0.0F;
}
