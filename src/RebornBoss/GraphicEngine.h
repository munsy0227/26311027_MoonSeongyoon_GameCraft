#pragma once
#include <glc2d.h>
#include <string>

class GraphicEngine
{
public:
	struct GAME_WINDOW
	{
		int x, y;
		int scnW, scnH;
		std::string name;
		bool bWindowed = true;
		float scnScale = 1.0F;
	};

	// 기본함수
public:
	int Init(const GraphicEngine::GAME_WINDOW& gameWindow);
	void Destroy();
	int Run();
	static int Update();
	static int Render();

public:
	void SetColor(unsigned long color);
};
