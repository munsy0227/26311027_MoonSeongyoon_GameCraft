#pragma once

class GameTime
{
public:
	int Init();
	long long beginTime		{};
	long long eplapsedTime	{};
	long long deltaTime		{};		// millisecond
};

