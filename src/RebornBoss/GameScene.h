#pragma once
#include <string>

enum GAME_PHASE: int
{
	SCENE_BEGIN = 0,
	SCENE_LOBBY,
	SCENE_PLAY,
	SCENE_PLAYRESULT,
};



class GameScene
{
public:
	GameScene();

	virtual int Init();
	virtual int Destroy();
	virtual int Update();
	virtual int Render();

protected:
	int m_font1	{-1};
	std::string  m_message;
};
