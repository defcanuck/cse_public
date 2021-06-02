#pragma once

#include "global/Values.h"
#include "global/Config.h"

#include <string>

namespace MainLoop
{	
	struct GameParams
	{
		GameParams()
			: gameSpeed(1.0f)
			, contentScale(1.0f)
			, fps(30)
			, absolutePath(true)
		{ }

		float32 gameSpeed;
		float32 contentScale;
		uint32 fps;


		bool absolutePath;
		std::string resourcePath;
	};


	void init();
	void initGL(const char* windowName, int32 width, int32 height);

	void config(
        const char* configFile,
        GameParams& params,
        int32 windowWidth,
        int32 windowHeight,
        float windowScale);
    
	void configPlatform(
        cs::Config* config,
        GameParams& params,
        int32 windowWidth,
        int32 windowHeight,
        float windowScale);

	void getScreenResolution(int32& width, int32& height, const char* configFile);

	void shutdown();
	void flip();

}

