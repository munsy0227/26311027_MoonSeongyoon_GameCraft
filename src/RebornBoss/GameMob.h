#pragma once
#include "GameCharacter.h"
class GameResources;

class GameMob : public GameCharacter
{
public:
    explicit GameMob(float x = 240.0F);
    void Update(float seconds);
    void Render(const GameResources& resources) const;
    bool Outside() const { return m_y > 616.0F; }

private:
    float m_animationSeconds{};
};
