#include "PCH.h"

#include "main/MainLoop.h"
#include "global/Values.h"

#include "main/MainLoop_Shared.h"
#include "scripting/ScriptNotification.h"

#include "os/LogManager.h"
#include "global/Timer.h"
#include "gfx/gl/OpenGL.h"
#include "gfx/RenderInterface.h"
#include "global/Stats.h"

#include "ecs/comp/ComponentHash.h"


namespace cs
{

	const uint32 kFramesPerSecond = 30;

	float32 MainEntry::total_ms = 0.0f;
	float32 MainEntry::process_ms = 0.0f;
	float32 MainEntry::render_ms = 0.0f;
	float32 MainEntry::game_speed = 1.0f;
	
	const int32 MainEntry::kDefaultWindowWidth = 1920;
	const int32 MainEntry::kDefaultWindowHeight = 1200;
    
    int gWindowWidth = MainEntry::kDefaultWindowWidth;
    int gWindowHeight = MainEntry::kDefaultWindowHeight;

	void testFunc()
	{
		log::print(cs::LogInfo, "Hello!");
	}

    
    void MainEntry::setScreenResolution(int32 width, int32 height)
    {
        gWindowWidth = width;
        gWindowHeight = height;
    }
    
	void MainEntry::getScreenResolution(int32& width, int32& height, const char* configFile)
	{
#if defined(CS_WINDOWS)
		MainLoop::getScreenResolution(width, height, configFile);
#else
        width = gWindowWidth;
        height = gWindowHeight;
#endif
    }
    
    void iPhoneDrawLoop(void* param)
    {
        MainEntry::FrameInfo* info = (MainEntry::FrameInfo*) param;
        info->entry->loop(info);
    }
    
    void MainEntry::setFrameBuffer()
    {
        RenderInterface::getInstance()->captureDefaultFrameBuffer();
    }
    
    void MainEntry::draw()
    {
        
        RenderInterface::getInstance()->beginFrame();
        
        Timer ms_render;
        ms_render.start();
        
		if (this->mainRenderFunc)
		{
			this->mainRenderFunc();
		}
        // RenderInterface::getInstance()->testFrame();

        
        MainEntry::render_ms = (float32) ms_render.getElapsed();
        
        RenderInterface::getInstance()->endFrame();
        
    }
    
    void MainEntry::process(float32 dt)
    {
        
        Timer ms_process;
        ms_process.start();
        
        if (this->mainProcessFunc)
            this->mainProcessFunc(dt);
        
        MainEntry::process_ms = (float32) ms_process.getElapsed();
    }
    
    void MainEntry::loop(FrameInfo* info)
    {
        // Keep track of the frame count
        int32 frame = 0;
		static Timer update;
        update.start();

		HighPrecisionTimer fps;

        // While application is running
        HighPrecisionTimer frame_timer;
        TimerCache::getInstance()->reset();
        
        float32 dt = MainEntry::total_ms * MainEntry::game_speed;
        // Update
        this->process(dt);
        
        // Draw
        this->draw();
        
        frame++;
        uint32 ticks = uint32(fps.getElapsed() * 1000);
        
        if (ticks < info->fpsInv)
        {
            // Sleep the remaining frame time
#if defined(CS_WINDOWS)
            SDL_Delay(info->fpsInv - ticks);
			//log::info(ticks);
#endif
        }

        
        MainEntry::total_ms = (float32) frame_timer.getElapsed();
        frame_timer.reset();
        
        // Update screen
		MainLoop::flip();
        
    }

	int MainEntry::runApp(const char* configFile, const char* windowName, const LaunchParams& launchParams)
	{

		EngineStats::init();

		MainLoop::GameParams params;
		
		MainLoop::init();
		MainLoop::config(
            configFile,
            params,
            launchParams.windowWidth,
            launchParams.windowHeight,
            launchParams.windowScale);

		MainEntry::game_speed = params.gameSpeed;
        RenderInterface::getInstance()->setContentScale(params.contentScale);

		FrameInfo info;
		info.desiredFrameRate = kFramesPerSecond;
		info.desiredMinDelta = 1.0f / float32(kFramesPerSecond);
		info.fpsInv = 1000 / kFramesPerSecond;

		if (params.fps > 0)
		{
			log::info("Setting frame rate to ", params.fps);
			info.desiredFrameRate = params.fps;
			info.desiredMinDelta = 1.0f / float32(params.fps);
			info.fpsInv = 1000 / params.fps;
		}
		
		initComponentHash();

		FileManager::getInstance()->setBasePath(params.resourcePath, params.absolutePath);

		MainEntry::getScreenResolution(gWindowWidth, gWindowHeight, configFile);
		log_info("Setting resolution to ", gWindowWidth, " x ", gWindowHeight);
		       
		
		MainLoop::initGL(windowName, gWindowWidth, gWindowHeight);

		if (mainInitFunc)
		{
			if (!mainInitFunc(gWindowWidth, gWindowHeight))
			{
				log::print(cs::LogError, "Init Failed!");
				return -1;
			}
		}
		else
		{
			log::print(cs::LogError, "No Init Function Defined");
			return -1;
		}

#if defined(CS_WINDOWS)

        info.width = gWindowWidth;
        info.height = gWindowHeight;
        info.entry = this;
        info.mainInputFunc = this->mainInputFunc;
        
		bool quit = false;
        while (!quit)
        {
           
            loop(&info);
          
            if (mainInputFunc && mainInputFunc(info.width, info.height))
                break;
            
            if (mainQuitFunc)
                quit = mainQuitFunc();
        }
   
		if (mainExitFunc)
			mainExitFunc();

		MainLoop::shutdown();
#endif

		return 0;
	}
    
    void MainEntry::postNotification(const char* notificationName)
    {
        std::string name(notificationName);
        ScriptNotification::getInstance()->pushNotification(notificationName);
    }

}
