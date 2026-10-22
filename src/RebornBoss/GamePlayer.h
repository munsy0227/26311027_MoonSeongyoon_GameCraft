#pragma once
#include "GameCharacter.h"

class GamePlayer : public GameCharacter
{
public:
	int Attack(GameCharacter* pMob, int damage) override;
};
