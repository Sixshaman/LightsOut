#include "Game\LightsOut.hpp"
#include "Util.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR nCmdLine, int nCmdShow)
{
	try
	{
		LightsOutApp theApp(hInstance);

		if(!theApp.InitAll())
		{
			return 1;
		}

		return theApp.RunApp();
	}
	catch (DXException e)
	{
		OutputDebugString(e.ToString().c_str());
		return 1;
	}
}