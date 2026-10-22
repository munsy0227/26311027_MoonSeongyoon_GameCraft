#include <iostream>
#include "App.h"

int main(int argc, char** argv)
{
	App app;
	if (0 > app.Init())
		return 0;

	auto hr = app.Run();
	//  hr
	{

	}


	return 0;
}
