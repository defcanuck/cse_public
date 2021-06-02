#pragma once

#include "global/Values.h"

struct SDL_Window;

struct LaunchParams
{
    int windowWidth;
    int windowHeight;
    float windowScale;
};

namespace cs
{

	typedef bool(*MainInputFunc)(int, int);
	typedef void(*MainProcessFunc)(float);
	typedef bool(*MainInitFunc)(int&, int&);
	typedef bool(*MainQuitFunc)();
	typedef void(*MainRenderFunc)();
	typedef void(*MainExitFunc)();

	void testFunc();

	struct MainEntry
	{
		MainEntry()
		{
			mainInputFunc = nullptr;
			mainProcessFunc = nullptr;
			mainInitFunc = nullptr;
			mainQuitFunc = nullptr;
			mainRenderFunc = nullptr;
			mainExitFunc = nullptr;
		}
        
        struct FrameInfo
        {
            MainEntry* entry;
            int width;
            int height;
            
            uint32 desiredFrameRate;
            float32 desiredMinDelta;
            uint32 fpsInv;
            
            MainInputFunc mainInputFunc;
        };
        
        static void setFrameBuffer();
        static void setScreenResolution(int32 width, int32 height);
		static void getScreenResolution(int32& width, int32& height, const char* configFile);

		static const int32 kDefaultWindowWidth;
		static const int32 kDefaultWindowHeight;

		MainInputFunc	mainInputFunc;
		MainProcessFunc	mainProcessFunc;
		MainInitFunc		mainInitFunc;
		MainQuitFunc		mainQuitFunc;
		MainRenderFunc	mainRenderFunc;
		MainExitFunc		mainExitFunc;

        void loop(FrameInfo* info);
        void draw();
        void process(float32 dt);
        void postNotification(const char* notificationName);
        
		int runApp(const char* configFile, const char* windowName, const LaunchParams& params);

		static float32 total_ms;
		static float32 process_ms;
		static float32 render_ms;
		static float32 game_speed;
	};
}
