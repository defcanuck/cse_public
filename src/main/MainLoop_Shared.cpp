#include "main/MainLoop_Shared.h"

#include "os/LogManager.h"


namespace MainLoop
{
	
    void config(
        const char* configFile,
        GameParams& params,
        int windowWidth,
        int windowHeight,
        float windowScale)
	{
		cs::Config* config = cs::ConfigManager::getInstance()->getConfig(configFile);
		if (!config || !config->isLoaded())
		{
			cs::log::error("Error - no config file found!");
			while (1)
			{
				// Loop until we exit
			}
		}

		std::string game_speed;
		if (config->getValue("game_speed", game_speed))
		{
			float32 new_game_speed = (float32)atof(game_speed.c_str());
			if (new_game_speed > 0.0f)
			{
				cs::log::info("Setting game speed to ", new_game_speed);
				params.gameSpeed = new_game_speed;
			}
		}

		std::string config_fps;
		if (config->getValue("fps", config_fps))
		{
			params.fps = atoi(config_fps.c_str());
		}

		configPlatform(config, params, windowWidth, windowHeight, windowScale);
	}
}
