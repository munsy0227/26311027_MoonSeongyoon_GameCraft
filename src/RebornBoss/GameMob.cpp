#include "GameMob.h"
#include "GameResources.h"

GameMob::GameMob(float x)
{
    m_x = x;
    m_y = -20.0F;
}

void GameMob::Update(float seconds)
{
    m_y += 60.0F * seconds;
    m_animationSeconds += seconds;
}

void GameMob::Render(const GameResources& resources) const
{
    const int column = static_cast<int>(m_animationSeconds / 0.12F) % 4;
    resources.DrawFrame(ImageId::PatrolFairy, column, 0, 32, 32, m_x - 16.0F, m_y - 16.0F);
}
