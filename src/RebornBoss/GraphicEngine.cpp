//link the 2d game library
#if defined(_DEBUG)
  #if defined(_M_X64) // 64-bit 아키텍처
    #pragma comment(lib, "glc2d_x64_debug.lib")
  #elif defined(_M_IX86) // 32-bit 아키텍처
    #pragma comment(lib, "glc2d_win32_debug.lib")
  #endif
#else
  #if defined(_M_X64)
    #pragma comment(lib, "glc2d_x64_release.lib")
  #elif defined(_M_IX86)
    #pragma comment(lib, "glc2d_win32_release.lib")
  #endif
#endif

#include "GraphicEngine.h"
#include <stdio.h>
#include "App.h"


int GraphicEngine::Update()
{
	return g_pApp->Update();
}

int GraphicEngine::Render()
{
	return g_pApp->Render();
}

void GraphicEngine::SetColor(unsigned long color)
{
	g2_SetClearColor(color);
}

int GraphicEngine::Init(const GAME_WINDOW& gameWindow)
{
	//sdk 초기화
	auto hr = g2_InitSdk();
	if (0 > hr)
		return -1;

	g2_SetFrameMove(GraphicEngine::Update);
	g2_SetRender(GraphicEngine::Render);


	// 윈도우 생성
	hr = g2_CreateWin(gameWindow.x
					, gameWindow.y
					, gameWindow.scnW, gameWindow.scnH
					, gameWindow.name.c_str()
					, gameWindow.bWindowed
					, gameWindow.scnScale);
	if (0 > hr)
		return -2;

	printf("Starting ...\n\n");
	return 0;
}

void GraphicEngine::Destroy()
{
	// 윈도우 해제
	g2_DestroyWin();
}

int GraphicEngine::Run()
{
	// 실행
	auto r = g2_Run();
	return r;
}
