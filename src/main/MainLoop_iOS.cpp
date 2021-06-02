#include "main/MainLoop_Shared.h"

#include "os/FileManager.h"
#include "global/Config.h"

#include "Platform_iOS.h"

#include <SDL.h>

using namespace cs;

namespace MainLoop
{
    void initGL(const char* windowName, int32 width, int32 height) { }
    void shutdown() { }
    void flip() { }

    void init()
	{
        std::string exePath = GetExecutablePath();
        cs::FileManager::getInstance()->setExecutablePath(exePath);
        
        /*
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            log::error("Error opening SDL Audio with error ", SDL_GetError());
        }
         */
	}
    
    void configPlatform(
        cs::Config* config,
        GameParams& params,
        int32 windowWidth,
        int32 windowHeight,
        float windowScale)
	{
		params.contentScale = float32(windowScale - 1.0f);
		params.resourcePath = GetBundlePath();
	}
    
    void getScreenResolution(int32& width, int32& height, const char* configFile)
    {
        
    }
}
