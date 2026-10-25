#pragma once
#include "GameCharacter.h"
class GameResources;

class GamePlayer : public GameCharacter
{
public:
    GamePlayer();
    void Update(float seconds, float directionX, float directionY, bool slow, bool shooting);
    void Render(const GameResources& resources) const;

private:
    float m_animationSeconds{};
    int m_row{};
    bool m_slow{};
};
