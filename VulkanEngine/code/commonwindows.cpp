

#include <windows.h>
#include <windowsx.h>
#include <string>
#include "commonwindows.h"
#include "util.h"

//handle the windows messages
LRESULT CALLBACK MessageHandler(HWND hwnd, UINT msg, WPARAM wP, LPARAM lP)
{
	InputInfo* input = (InputInfo*)GetWindowLongPtr(hwnd, 0);
	switch (msg)
	{
		case WM_CREATE:
			{
				CREATESTRUCTW* cs = (CREATESTRUCTW*)lP;
				SetWindowLongPtrW(hwnd, 0, (LONG_PTR)cs->lpCreateParams);
			}
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			{
				input->running = false;
			}
			break;
		case WM_IME_KEYUP:
		case WM_KEYUP:
			{
				//only works for windows since the keys are based on windows key codes
				//would have to use a switch for other platforms but for now this seems at least somewhat elegant
				if (wP < InputCodesSize)
				{
					input->keys[wP] = false;
				}
			}
			break;
		case WM_IME_KEYDOWN:
		case WM_KEYDOWN:
			{
				//only works for windows since the keys are based on windows key codes
				//would have to use a switch for other platforms but for now this seems at least somewhat elegant
				if (wP < InputCodesSize)
				{
					input->keys[wP] = true;
				}
			}
			break;
		case WM_MOUSEMOVE:
		case WM_NCMOUSEHOVER:
		case WM_NCMOUSELEAVE:
		case WM_MOUSEHOVER:
			{
				BOOL tracked = TrackMouseEvent(&input->mouseEvent);
				if(tracked == 0x1)
				{
					input->mouseInWindow = true;
				}else
				{
					input->mouseInWindow = false;
					
				}

				input->lastMousePos = input->mousePos;
				input->mousePos = glm::vec2((float)GET_X_LPARAM(lP), (float)GET_Y_LPARAM(lP));
			}
			break;
		case WM_LBUTTONDOWN:
		{
			input->keys[lMouse] = true;
		}
		break;
		case WM_LBUTTONUP:
		{
			input->keys[lMouse] = false;
		}
		break;
		default:
			{
				return DefWindowProc(hwnd, msg, wP, lP);
			}
	}
	return 0;
}


WindowInfo NewWindowInfo(const char* appName, void* pointer, uint32_t clientWidth, uint32_t clientHeight)
{

	WindowInfo windowInfo;

	WNDCLASS wc = {};

	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MessageHandler;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = appName;
	wc.lpszMenuName = appName;
	wc.cbWndExtra = sizeof(void*);


	ATOM result = 0;
	result = RegisterClass(&wc);

	Assert(result != 0, "could not register windowclass");


	//TODO calculate windowrect from clientrect
	uint32_t windowWidth = clientWidth;
	uint32_t windowHeight = clientHeight;

	HWND windowHandle = CreateWindow(appName,
		appName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		wc.hInstance,
		pointer);

	Assert(windowHandle != nullptr, "could not create a windows window");

	ShowWindow(windowHandle, SW_SHOW);

	windowInfo.AppName = appName;
	windowInfo.windowHandle = windowHandle;
	windowInfo.clientWidth = clientWidth;
	windowInfo.clientHeight = clientHeight;
	windowInfo.exeHandle = GetModuleHandle(nullptr);

	return windowInfo;
}


void DestroyWindowInfo(WindowInfo* windowInfo)
{
	DestroyWindow(windowInfo->windowHandle);
	windowInfo = {};
}


File OpenFile(std::string fileName)
{
	File file = {};

	HANDLE fileHandle = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			Assert(fileSize.QuadPart < (int64_t)pow(2, 32), "file too big");
			uint32_t fileSize32 = fileSize.QuadPart;
			file.data = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(file.data)
			{
				DWORD bytesRead;
				if(ReadFile(fileHandle, file.data, fileSize32, &bytesRead, 0) && fileSize32 == bytesRead)
				{
					file.size = fileSize32;
				}
				else
				{
					VirtualFree(file.data, 0, MEM_RELEASE);
					file.data = nullptr;
				}
			}

		}
	}
	Assert(file.data != nullptr, "could not read file");
	return file;	
}


TimerInfo NewTimerInfo()
{
	TimerInfo timerInfo = {};
	LARGE_INTEGER clocksPerSec;
	QueryPerformanceFrequency(&clocksPerSec);
	timerInfo.clocksPerSec = clocksPerSec.QuadPart;

	TIMECAPS timeCaps;
	timeGetDevCaps(&timeCaps, sizeof(timeCaps));

	timeBeginPeriod(timeCaps.wPeriodMin);
	timerInfo.clockReslution = timeCaps.wPeriodMin;
	return timerInfo;
}


int64_t GetClockCount()
{
	LARGE_INTEGER clockCount;
	QueryPerformanceCounter(&clockCount);
	return clockCount.QuadPart;
}



uint64_t GetAvgFps(const TimerInfo* timerInfo)
{
	uint64_t avgFps = 0;
	for (uint32_t i = 0; i < 10; i++)
	{
		avgFps += timerInfo->framesPerSec[i];
	}
	avgFps /= 10;

	return avgFps;
}

InputInfo NewInputInfo(const WindowInfo* windowInfo)
{
	InputInfo iInfo = {};
	iInfo.running = true;
	iInfo.mouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
	iInfo.mouseEvent.dwFlags = TME_HOVER | TME_LEAVE;
	iInfo.mouseEvent.hwndTrack = windowInfo->windowHandle;
	iInfo.mouseEvent.dwHoverTime = HOVER_DEFAULT;
	return iInfo;
}

void Tick(TimerInfo* timerInfo)
{
	timerInfo->startClocks = GetClockCount();
	timerInfo->numFrames++;
}


void Tock(TimerInfo* timerInfo)
{
	timerInfo->endClocks = GetClockCount();
	int64_t frameClocks = timerInfo->endClocks - timerInfo->startClocks;
	timerInfo->frameTimeMilliSec = (1000 * frameClocks) / timerInfo->clocksPerSec;
	timerInfo->framesPerSec[timerInfo->numFrames % 10] = (float)timerInfo->clocksPerSec / (float)frameClocks;
}


void Sleep(TimerInfo* timerInfo, int32_t desiredFps)
{
	int32_t desiredFrameDurationMilliSec = 1000 / desiredFps;
	int32_t frameTimeRemainingMilliSec = desiredFrameDurationMilliSec - timerInfo->frameTimeMilliSec;
	if( frameTimeRemainingMilliSec > 1 && frameTimeRemainingMilliSec < desiredFrameDurationMilliSec)
	{
		//frameRemaindertime is greater than clock resolution AND positive, so it can sleep.
		//calculate the sleeptime which is the lowest granularity value of frameRemaindertime that will still guarantee that
		//the sleep function wont accidentally sleep over the desireddeltaframetime.
		uint64_t sleepTimeMilliSec = frameTimeRemainingMilliSec - (frameTimeRemainingMilliSec % timerInfo->clockReslution);
		Sleep((uint32_t)sleepTimeMilliSec);
		//update the timer since the frames duration has changed
		Tock(timerInfo);
	}
	else if(frameTimeRemainingMilliSec < 0)
	{
		//Message("COULD NOT REACH DESIRED FPS");
		//TODO heavier error warning or something?
	}else
	{
		//do nothing, just barely met framerate
	}
}

void DestroyTimerInfo(TimerInfo* timerInfo)
{
	timeEndPeriod(timerInfo->clockReslution);
}