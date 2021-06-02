#include "main/MainLoop_Shared.h"

#include <SDL.h>

#include "os/FileManager.h"
#include "global/Config.h"

#include <string>

namespace
{
	SDL_GLContext gContext;
	SDL_Window *gScreen;
}

void MainLoop::init()
{
	cs::log::print(cs::LogInfo, "Initializing SDL.");

	if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)) {
		cs::log::print(cs::LogError, "Could not initialize SDL: ", SDL_GetError());
		exit(-1);
	}

	cs::log::print(cs::LogInfo, "SDL initialized.");

	// Initialize the SDL library 
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		cs::log::print(cs::LogError, "Couldn't initialize SDL: ", SDL_GetError());
		exit(-1);
	}

	char* path = SDL_GetBasePath();
	if (!path)
	{
		log_error("ERROR BAD EXE PATH!  FATAL!");
		exit(-1);
	}

	cs::FileManager::setExecutablePath(path);
	SDL_free(path);

	// Clean up on exit 
	atexit(SDL_Quit);


}

void MainLoop::initGL(const char* windowName, int32 width, int32 height)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	int displayFlags = SDL_WINDOW_OPENGL;


	// Create window
	gScreen = SDL_CreateWindow(
		windowName,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		displayFlags);

	if (gScreen == nullptr)
	{
		cs::log::print(cs::LogError, "Couldn't set 640x480x8 video mode: ", SDL_GetError());
		exit(1);
	}

	// force fullscreen off
	SDL_SetWindowFullscreen(gScreen, 0);

	// Create context
	gContext = SDL_GL_CreateContext(gScreen);
	if (gContext == nullptr)
	{
		cs::log::print(cs::LogError, "OpenGL context could not be created! SDL Error: ", SDL_GetError());
		exit(1);
	}

	if (SDL_GL_SetSwapInterval(1) < 0)
	{
		printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
	}

	SDL_StartTextInput();


}

void MainLoop::configPlatform(
	cs::Config* config, 
	GameParams& params, 
	int32 windowWidth,
	int32 windowHeight,
	float32 windowScale)
{
	
	if (!config->getValue("resourcePath", params.resourcePath))
	{
		log_error("ERROR NO PATH DEFINED! FATAL!");
		exit(-1);
	}

	params.absolutePath = config->exists("absolutePath");

	if (config->exists("content_scale"))
	{
		std::string contentScale;
		config->getValue("content_scale", contentScale);
		params.contentScale = (float) atof(contentScale.c_str());
	}

}

void MainLoop::getScreenResolution(int32& width, int32& height, const char* configFile)
{
	cs::Config* config = cs::ConfigManager::getInstance()->getConfig(configFile);
	assert(config);
	std::string resolution;
	if (config->getValue("window_resolution", resolution))
	{
		size_t xpos = resolution.find_last_of("x");
		if (xpos != std::string::npos)
		{
			width = atoi(resolution.substr(0, xpos).c_str());
			height = atoi(resolution.substr(xpos + 1, std::string::npos).c_str());
		}
	}
}


void MainLoop::shutdown()
{
	// Cleanup
	SDL_StopTextInput();
	SDL_GL_DeleteContext(gContext);
	SDL_DestroyWindow(gScreen);
	SDL_Quit();
}

void MainLoop::flip()
{
	SDL_GL_SwapWindow(gScreen);
}