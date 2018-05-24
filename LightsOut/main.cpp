#include "LightsOut.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR nCmdLine, int nCmdShow)
{
	LightsOutApp theApp(hInstance);

	if(!theApp.InitAll())
	{
		return 1;
	}

	return theApp.RunApp();
}