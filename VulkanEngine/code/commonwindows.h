#pragma once

#define GLM_FORCE_RADIANS
#include <windows.h>
#include <glm/glm.hpp>


struct File
{
	void* data;
	uint32_t size;
};

//unless otherwise specified, the button codes here will be exactly the same as by windows.
//the unused buttons may be remapped to save space in the array

	enum InputCodes
	{
		null = 0x0,
		lMouse = 0x1,
		rMouse = 0x2,
		cancel = 0x3,
		mMouse = 0x4,
		x1Mouse = 0x5,
		x2Mouse = 0x6,
		unused7 = 0x7, //TODO use for something else?
		backSpace = 0x8,
		tab = 0x9,
		unuseda = 0xa,
		unusedb = 0xb,
		clear = 0xc,
		enter = 0xd,
		unusede = 0xe,
		unusedf = 0xf,
		shift = 0x10,
		ctrl = 0x11,
		alt = 0x12,
		pause = 0x13,
		caps = 0x14,
		unused15 = 0x15,
		unused16 = 0x16,
		unused17 = 0x17,
		unused18 = 0x18,
		unused19 = 0x19,
		unused1a = 0x1a,
		esc = 0x1b,
		//couple unused here
		space = 0x20,
		pageup = 0x21,
		pagedown = 0x22,
		end = 0x23,
		home = 0x24,
		left = 0x25,
		up = 0x26,
		right = 0x27,
		down = 0x28,
		sel = 0x29, // NOTE cant use select because its already used for a function?
		print = 0x2a,
		execute = 0x2b,
		printScreen = 0x2c,
		insert = 0x2d,
		del = 0x2e, //delete is already used
		help = 0x2f,
		key0 = 0x30,
		key1 = 0x31,
		key2 = 0x32,
		key3 = 0x33,
		key4 = 0x34,
		key5 = 0x35,
		key6 = 0x36,
		key7 = 0x37,
		key8 = 0x38,
		key9 = 0x39,
		//some unused here
		keyA = 0x41,
		keyB = 0x42,
		keyC = 0x43,
		keyD = 0x44,
		keyE = 0x45,
		keyF = 0x46,
		keyG = 0x47,
		keyH = 0x48,
		keyI = 0x49,
		keyJ = 0x4a,
		keyK = 0x4b,
		keyL = 0x4c,
		keyM = 0x4d,
		keyN = 0x4e,
		keyO = 0x4f,
		keyP = 0x50,
		keyQ = 0x51,
		keyR = 0x52,
		keyS = 0x53,
		keyT = 0x54,
		keyU = 0x55,
		keyV = 0x56,
		keyW = 0x57,
		keyX = 0x58,
		keyY = 0x59,
		keyZ = 0x5a,

		//KEEP THIS AT THE BOTTOM, ALWAYS.
		InputCodesSize
	};

struct WindowInfo
{
	const char* AppName;
	HWND windowHandle;
	HINSTANCE exeHandle;
	uint32_t clientWidth, clientHeight;
};


struct InputInfo
{
	bool running;
	bool keys[InputCodesSize];
	glm::vec2 mousePos;
	glm::vec2 lastMousePos;
	bool mouseInWindow;
	TRACKMOUSEEVENT mouseEvent;
};

struct TimerInfo
{
	uint64_t clocksPerSec;
	uint32_t clockReslution;
	float framesPerSec[10];
	uint64_t numFrames;

	int64_t startClocks;
	int64_t endClocks;
	int64_t frameTimeMilliSec;
};

//returns an InputInfo struct, and requires a valid/initialized windowInfo struct for its hwnd.
InputInfo NewInputInfo(const WindowInfo* windowInfo);

//create a windows window, pass a pointer to a struct for input events
//returns a handle to the created window.
WindowInfo NewWindowInfo(const char* appName, void* pointer, uint32_t clientWidth, uint32_t clientHeight);


//returns a struct containing the pointer to the data and data size of the file specified by its absolute path
File OpenFile(std::string fileName);

//destroy the windowinfo
//really just a wrapper to destroy the window itself
void DestroyWindowInfo(WindowInfo* windowInfo);


//create a new timerinfo struct, this function fills in constants such as the clock freqency
//provided by windows etc
TimerInfo NewTimerInfo();

//returns the average framerate of the last 10 frames
uint64_t GetAvgFps(const TimerInfo* timerInfo);

//returns the number of clock cycles since process startup
int64_t GetClockCount();

//designed to be used along with tick/tock functions to limit the framerate of the application
//use tick at the beginning of the frame and tock at the end of the frame then call sleep
//with the desired fps to limit the framerate.
//TODO this function is a little unreliable when there is a very small amount of work to do in a 
//single frame, consider changing the units to microseconds instead of milliseconds?
void Sleep(TimerInfo* timerInfo, int32_t desiredFps);

//update the starting time and increment the number of frames
void Tick(TimerInfo* timerInfo);

//update the rest of the timerinfo struct since the end of the desired block of code has been reached
void Tock(TimerInfo* timerInfo);

void DestroyTimerInfo(TimerInfo* timerInfo);