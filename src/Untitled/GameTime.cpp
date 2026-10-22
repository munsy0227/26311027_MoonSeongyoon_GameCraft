#include "GameTime.h"
#include <iostream>
#include <thread>
#include <glc2d.h>
using namespace std;

int GameTime::Init()
{
	begintime		= g2_TimeGetTime();
	eplapsedTime	= {};
	deltaTime		= {};			// millisecond
	return 0;
}

int GameTime::Update()
{
	// test
	this_thread::sleep_for(chrono::milliseconds(10));	// 0.01√  sleep

	auto currentTime = g2_TimeGetTime();
	auto prevTime = eplapsedTime;
	eplapsedTime = currentTime - begintime;
	deltaTime = eplapsedTime - prevTime;			// millisecond
	cout << "GameTime::Update() - eplapsedTime: " << eplapsedTime << ", deltaTime: " << deltaTime << endl;
	return 0;
}
