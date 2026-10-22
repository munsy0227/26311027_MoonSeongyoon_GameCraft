#pragma once

class GameCharacter
{
public:
	virtual int Damage(int damage);
	virtual int Attack(GameCharacter* pMob, int damage)
	{
		pMob->Damage(damage);
		return 0;
	}
};
