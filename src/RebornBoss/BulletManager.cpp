#include "BulletManager.h"
#include "GameResources.h"
#include <algorithm>
#include <cmath>
#include <numbers>

bool BulletManager::Update(float seconds, bool shooting, float playerX, float playerY)
{
    for (auto& bullet : m_bullets)
        if (bullet.active)
            bullet.Update(seconds);
    RemoveInactive();
    m_shotTimer -= seconds;
    if (!shooting)
    {
        m_shotTimer = std::max(0.0F, m_shotTimer);
        return false;
    }
    bool fired = false;
    while (m_shotTimer <= 0.000001F)
    {
        Add({playerX, playerY - 28.0F});
        m_shotTimer += 0.12F;
        fired = true;
    }
    return fired;
}

void BulletManager::Add(GameBullet bullet)
{
    m_bullets.push_back(bullet);
}

void BulletManager::AddAimedFan(float x, float y, float targetX, float targetY,
                                int count, float stepDegrees, float speed)
{
    const float dx = targetX - x;
    const float dy = targetY - y;
    if (std::hypot(dx, dy) <= 0.001F)
        return;
    const float direction = std::atan2(dy, dx);
    const float step = stepDegrees * std::numbers::pi_v<float> / 180.0F;
    // Snapshot the target once; released bullets keep this velocity.
    for (int i = 0; i < count; ++i)
    {
        const float angle = direction + (i - (count - 1) * 0.5F) * step;
        Add({x, y, std::sin(angle) * speed, std::cos(angle) * speed, BulletTeam::Enemy});
    }
}

void BulletManager::ClearEnemy()
{
    std::erase_if(m_bullets, [](const GameBullet& bullet) { return bullet.team == BulletTeam::Enemy; });
}

void BulletManager::RemoveInactive()
{
    std::erase_if(m_bullets, [](const GameBullet& bullet) { return !bullet.active || bullet.Outside(); });
}

void BulletManager::Render(const GameResources& resources) const
{
    for (const auto& bullet : m_bullets)
    {
        if (!bullet.active)
            continue;
        if (bullet.team == BulletTeam::Player)
            resources.Draw(ImageId::PlayerShot, bullet.x - 6.0F, bullet.y - 12.0F);
        else
            resources.Draw(bullet.orb ? ImageId::EnemyOrb : ImageId::EnemyPetal,
                           bullet.x - 8.0F, bullet.y - (bullet.orb ? 8.0F : 12.0F));
    }
}

void BulletManager::Clear()
{
    m_bullets.clear();
    m_shotTimer = 0.0F;
}
