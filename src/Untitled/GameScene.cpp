#include "GameScene.h"
#include <glc2d.h>

int GameScene::Init()
{
	// Windows에 설치된 글꼴을 사용하므로 폰트 파일을 배포하지 않습니다.
	m_font1 = g2_FontCreate("Malgun Gothic", 64);
	if (0 > m_font1)
		return -1;
	return 0;
}

int GameScene::Update()
{
	return 0;
}

int GameScene::Render()
{
	g2_FontDrawText(m_font1, { 10, 10 }, 0xFFFF00FF, "Game Scene");
	return 0;
}
