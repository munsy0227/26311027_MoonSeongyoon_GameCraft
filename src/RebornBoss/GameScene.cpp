#include <glc2d.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include "GameScene.h"
#include "GameInput.h"

GameScene::GameScene()
{
	m_message = "Game Scene";
}

int GameScene::Init()
{
	// Windows에 설치된 글꼴을 사용하므로 폰트 파일을 배포하지 않습니다.
	m_font1 = g2_FontCreate("Malgun Gothic", 64);
	if (0 > m_font1)
		return -1;
	return 0;
}

int GameScene::Destroy()
{
	return 0;
}

int GameScene::Update()
{
	auto keyEvent = GetGameInput()->GetKeyState(VK_RIGHT);
	if (keyEvent == EInput::EINPUT_DOWN)
	{
		// next window
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	return 0;
}

int GameScene::Render()
{
	g2_FontDrawText(m_font1, { 10, 10 }, 0xFFFF00FF, m_message.c_str());
	return 0;
}
