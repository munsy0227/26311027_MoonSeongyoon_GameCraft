#include "GameTime.h"
#include <chrono>
using namespace std;


int GameTime::Init()
{
	chrono::system_clock::now();


	beginTime	 =  chrono::system_clock::now();
	eplapsedTime =	{};
	deltaTime	 =	{};
}