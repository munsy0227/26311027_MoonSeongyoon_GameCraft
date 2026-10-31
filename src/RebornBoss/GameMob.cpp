#include "GameMob.h"
#include "BulletManager.h"
#include "GameResources.h"

GameMob::GameMob(float x)
{
    m_x = x;
    m_y = -20.0F;
    m_health = 3;
}

void GameMob::Update(float seconds)
{
    if (Dead())
        return;
    m_y += 60.0F * seconds;
    m_animationSeconds += seconds;
}

void GameMob::Update(float seconds, BulletManager& bullets)
{
    Update(seconds);
    if (Dead() || Outside())
        return;
    m_shotTimer -= seconds;
    while (m_shotTimer <= 0.000001F)
    {
        bullets.Add({m_x, m_y + 16.0F, 110.0F, 0.0F, BulletTeam::Enemy});
        m_shotTimer += 1.5F;
    }
}

void GameMob::Render(const GameResources& resources) const
{
    if (Dead())
        return;
    const int column = static_cast<int>(m_animationSeconds / 0.12F) % 4;
    resources.DrawFrame(ImageId::PatrolFairy, column, 0, 32, 32, m_x - 16.0F, m_y - 16.0F);
}
