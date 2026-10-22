#pragma once
#include "GameCharacter.h"

class GameMob : public GameCharacter
{
public:
	int Attack(GameCharacter* pMob, int damage);
};
