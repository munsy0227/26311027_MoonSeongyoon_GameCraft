#include "GameCharacter.h"
#include "GamePlayer.h"

int GamePlayer::Attack(GameCharacter* pMob, int damage)
{
	// 내일을 처리하고

	//
	pMob->Damage(damage);
	return 0;
}
