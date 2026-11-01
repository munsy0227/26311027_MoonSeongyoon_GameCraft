#include "GameMob.h"
#include "BulletManager.h"
#include "GameResources.h"
#include <algorithm>
#include <cmath>
#include <numbers>

GameMob::GameMob(float x, MobType type) : m_type(type), m_spawnX(x)
{
    m_x = x;
    m_y = -20.0F;
    m_health = type == MobType::Guardian ? 5 : 3;
    m_shotTimer = type == MobType::Patrol ? 1.4F : type == MobType::Familiar ? 1.6F : 2.0F;
}

void GameMob::Update(float seconds)
{
    if (Dead())
        return;
    m_animationSeconds += seconds;
    const float elapsed = static_cast<float>(m_animationSeconds);
    if (m_type == MobType::Familiar)
    {
        m_x = std::clamp(m_spawnX + 48.0F * std::sin(2.0F * std::numbers::pi_v<float> * elapsed / 3.0F),
                         32.0F, 448.0F);
        m_y = -20.0F + 50.0F * elapsed;
    }
    else if (m_type == MobType::Guardian)
        m_y = elapsed < 2.0F ? -20.0F + 85.0F * elapsed
                            : elapsed <= 4.5F ? 150.0F : 150.0F + 85.0F * (elapsed - 4.5F);
    else
        m_y = -20.0F + 60.0F * elapsed;
}

void GameMob::Update(float seconds, BulletManager& bullets, float playerX, float playerY)
{
    Update(seconds);
    if (Dead() || Outside())
        return;
    m_shotTimer -= seconds;
    while (m_shotTimer <= 0.000001F)
    {
        const float sourceY = m_y + RenderSize() * 0.5F;
        const float dx = playerX - m_x;
        const float dy = playerY - sourceY;
        const bool stationary = m_type != MobType::Guardian
            || (m_animationSeconds >= 2.0 - 0.00001 && m_animationSeconds <= 4.5 + 0.00001);
        if (stationary && dy > 0.0F && dx * dx + dy * dy >= 96.0F * 96.0F)
        {
            const int count = m_type == MobType::Patrol ? 3 : m_type == MobType::Familiar ? 1 : 5;
            const float spacing = m_type == MobType::Guardian ? 12.0F : 8.0F;
            const float speed = m_type == MobType::Patrol ? 90.0F : m_type == MobType::Familiar ? 85.0F : 80.0F;
            bullets.AddAimedFan(m_x, sourceY, playerX, playerY, count, spacing, speed);
        }
        // Unsafe or moving volleys still consume their slot instead of building a burst.
        m_shotTimer += m_type == MobType::Patrol ? 1.4F : m_type == MobType::Familiar ? 1.6F : 2.0F;
    }
}

void GameMob::Render(const GameResources& resources) const
{
    if (Dead())
        return;
    const int column = static_cast<int>(m_animationSeconds / 0.12F) % 4;
    const ImageId image = m_type == MobType::Patrol ? ImageId::PatrolFairy
                        : m_type == MobType::Familiar ? ImageId::MistFamiliar : ImageId::BarrierGuardian;
    const int cell = CellSize();
    const float halfSize = RenderSize() * 0.5F;
    resources.DrawFrame(image, column, 0, cell, cell, m_x - halfSize, m_y - halfSize, RenderScale);
}
