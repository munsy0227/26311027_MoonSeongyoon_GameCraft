#pragma once

struct GameTime
{
	int Init();
	int Update();
	long long begintime	{};
	long long eplapsedTime	{};
	long long deltaTime	{};			// millisecond
};
