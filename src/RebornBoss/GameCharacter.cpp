#include "GameCharacter.h"
#include <algorithm>

int GameCharacter::Damage(int damage)
{
    const int applied = std::min(m_health, std::max(0, damage));
    m_health -= applied;
    return applied;
}
