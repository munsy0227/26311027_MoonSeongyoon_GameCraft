#include "GameBossMob.h"
#include "BulletManager.h"
#include "GameResources.h"
#include <cmath>
#include <numbers>

GameBossMob::GameBossMob()
{
    m_x = 240.0F;
    m_y = 120.0F;
    m_health = MaxHealth;
}

void GameBossMob::Update(float seconds, BulletManager& bullets)
{
    if (Dead())
        return;
    m_animationSeconds += seconds;
    m_shotTimer -= seconds;
    while (m_shotTimer <= 0.000001F)
    {
        const int count = m_phaseTwo ? 24 : 7;
        const float speed = m_phaseTwo ? 120.0F : 140.0F;
        for (int i = 0; i < count; ++i)
        {
            const float angle = m_phaseTwo
                ? 2.0F * std::numbers::pi_v<float> * i / count
                : std::numbers::pi_v<float> * (0.25F + i / 12.0F);
            GameBullet bullet{m_x, m_y, std::sin(angle) * speed,
                              std::cos(angle) * speed, BulletTeam::Enemy};
            bullet.orb = m_phaseTwo;
            bullets.Add(bullet);
        }
        m_shotTimer += m_phaseTwo ? 1.5F : 1.2F;
    }
}

bool GameBossMob::AdvancePhase(BulletManager& bullets)
{
    if (Dead() || m_phaseTwo || m_health > MaxHealth / 2)
        return false;
    m_phaseTwo = true;
    m_shotTimer = 1.5F;
    bullets.ClearEnemy();
    return true;
}

void GameBossMob::Render(const GameResources& resources) const
{
    if (Dead())
        return;
    const int column = static_cast<int>(m_animationSeconds / 0.09F) % 4;
    resources.DrawFrame(ImageId::Lillian, column, 3, 64, 64, m_x - 32.0F, m_y - 32.0F);
}
