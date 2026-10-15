#pragma once
class App
{
public:
	App();
	virtual ~App();

	int init();
	int Destroy();
	int Update();
	int Render();
};

