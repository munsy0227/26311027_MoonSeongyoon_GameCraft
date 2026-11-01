#include "GameBossMob.h"
#include "BulletManager.h"
#include "GameResources.h"
#include <cmath>
#include <numbers>

GameBossMob::GameBossMob()
{
    m_x = 240.0F;
    m_y = BaseY;
    m_health = MaxHealth;
}

void GameBossMob::Update(float seconds, BulletManager& bullets, float playerX, float playerY)
{
    if (Dead())
        return;
    m_animationSeconds += seconds;
    const float elapsed = static_cast<float>(m_animationSeconds);
    m_x = 240.0F + 100.0F * std::sin(2.0F * std::numbers::pi_v<float> * elapsed / 10.5F);
    m_y = BaseY + 12.0F * std::sin(2.0F * std::numbers::pi_v<float> * elapsed / 7.5F);
    m_shotTimer -= seconds;
    while (m_shotTimer <= 0.000001F)
    {
        if (!m_phaseTwo)
            bullets.AddAimedFan(m_x, m_y, playerX, playerY, 7, 8.0F, 155.0F);
        else
        {
            for (int i = 0; i < 36; ++i)
            {
                const float angle = m_ringAngle + 2.0F * std::numbers::pi_v<float> * i / 36;
                GameBullet bullet{m_x, m_y, std::sin(angle) * 125.0F,
                                  std::cos(angle) * 125.0F, BulletTeam::Enemy};
                bullet.orb = true;
                bullets.Add(bullet);
            }
            if (m_aimNextRing)
                bullets.AddAimedFan(m_x, m_y, playerX, playerY, 5, 8.0F, 175.0F);
            m_aimNextRing = !m_aimNextRing;
            m_ringAngle = std::fmod(m_ringAngle + std::numbers::pi_v<float> / 30.0F,
                                   2.0F * std::numbers::pi_v<float>);
        }
        m_shotTimer += m_phaseTwo ? 1.0F : 0.85F;
    }
}

bool GameBossMob::AdvancePhase(BulletManager& bullets)
{
    if (Dead() || m_phaseTwo || m_health > MaxHealth / 2)
        return false;
    m_phaseTwo = true;
    m_shotTimer = 1.0F;
    bullets.ClearEnemy();
    return true;
}

void GameBossMob::Render(const GameResources& resources) const
{
    if (Dead())
        return;
    const int column = static_cast<int>(m_animationSeconds / 0.09F) % 4;
    resources.DrawFrame(ImageId::Lillian, column, 3, 64, 64,
                        m_x - RenderSize * 0.5F, m_y - RenderSize * 0.5F, RenderScale);
}
