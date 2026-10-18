#include <iostream>
#include "App.h"
using namespace std;


App* g_pApp;

int main(int argc, char** argv)
{
	App app;
	g_pApp = &app;

	if (0 > g_pApp->Init())
		return 0;
	
	
	auto r = g_pApp->Run();
	//  hr 에러 처리 코드
	{

	}

	cout << "Untitled \n";

	return 0;
}