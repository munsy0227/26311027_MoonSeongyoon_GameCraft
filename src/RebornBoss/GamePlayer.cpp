#include "GamePlayer.h"
#include "GameResources.h"
#include <algorithm>
#include <cmath>

GamePlayer::GamePlayer()
{
    m_x = 240.0F;
    m_y = 520.0F;
    m_health = 3;
}

void GamePlayer::Update(float seconds, float directionX, float directionY, bool slow, bool shooting)
{
    m_invulnerabilitySeconds = std::max(0.0F, m_invulnerabilitySeconds - seconds);
    const float length = std::hypot(directionX, directionY);
    const float speed = slow ? 100.0F : 240.0F;
    if (length > 0.0F)
    {
        const float velocityLength = std::hypot(m_velocityX, m_velocityY);
        if (velocityLength > speed)
        {
            m_velocityX *= speed / velocityLength;
            m_velocityY *= speed / velocityLength;
        }
        const float targetX = directionX * speed / length;
        const float targetY = directionY * speed / length;
        const float decay = std::exp(-seconds / 0.045F);
        // Exact integration keeps the short acceleration consistent across frame rates.
        const float response = 0.045F * (1.0F - decay);
        m_x += targetX * seconds + (m_velocityX - targetX) * response;
        m_y += targetY * seconds + (m_velocityY - targetY) * response;
        m_velocityX = targetX + (m_velocityX - targetX) * decay;
        m_velocityY = targetY + (m_velocityY - targetY) * decay;
    }
    else
        m_velocityX = m_velocityY = 0.0F;
    constexpr float halfWidth = RenderWidth * 0.5F;
    constexpr float halfHeight = RenderHeight * 0.5F;
    m_x = std::clamp(m_x, halfWidth, 480.0F - halfWidth);
    m_y = std::clamp(m_y, halfHeight, 600.0F - halfHeight);
    if ((m_x == halfWidth && m_velocityX < 0.0F) || (m_x == 480.0F - halfWidth && m_velocityX > 0.0F))
        m_velocityX = 0.0F;
    if ((m_y == halfHeight && m_velocityY < 0.0F) || (m_y == 600.0F - halfHeight && m_velocityY > 0.0F))
        m_velocityY = 0.0F;
    const int row = m_velocityX < -1.0F ? 1 : m_velocityX > 1.0F ? 2 : shooting ? 3 : 0;
    if (row != m_row)
        m_animationSeconds = 0.0F;
    m_row = row;
    m_animationSeconds += seconds;
    m_slow = slow;
}

int GamePlayer::Damage(int damage)
{
    if (m_invulnerabilitySeconds > 0.0F)
        return 0;
    const int applied = GameCharacter::Damage(damage);
    if (applied > 0)
        m_invulnerabilitySeconds = 1.5F;
    return applied;
}

bool GamePlayer::UseBomb()
{
    if (m_bombs == 0 || Dead())
        return false;
    --m_bombs;
    m_invulnerabilitySeconds = std::max(m_invulnerabilitySeconds, 1.0F);
    return true;
}

bool GamePlayer::Heal()
{
    if (Dead() || m_health >= MaxHealth)
        return false;
    ++m_health;
    return true;
}

bool GamePlayer::AddBomb()
{
    if (Dead() || m_bombs >= MaxBombs)
        return false;
    ++m_bombs;
    return true;
}

bool GamePlayer::AddPower()
{
    if (Dead() || m_powerPoints >= MaxPowerPoints)
        return false;
    ++m_powerPoints;
    return true;
}

void GamePlayer::Render(const GameResources& resources) const
{
    if (Dead())

        return;
    const float frameSeconds = m_row == 0 ? 0.12F : m_row == 3 ? 0.09F : 0.10F;
    const int column = static_cast<int>(m_animationSeconds / frameSeconds) % 4;
    if (m_invulnerabilitySeconds <= 0.0F || static_cast<int>(m_invulnerabilitySeconds * 12.0F) % 2 == 0)
        resources.DrawFrame(ImageId::Leon, column, m_row, 32, 48,
                            m_x - RenderWidth * 0.5F, m_y - RenderHeight * 0.5F, RenderScale);
    if (m_slow)
        resources.Draw(ImageId::Hitbox, m_x - 6.0F, m_y - 6.0F);
}
