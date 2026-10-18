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


	auto hr = g_pApp->Run();
	//  hr
	{

	}


	return 0;
}
