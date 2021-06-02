#pragma once

#include "game/Context.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(GameContext, Context)
	public:
		GameContext(const std::string& name, const RectI& rect)
			: Context(name, rect)
		{ }

		virtual bool loadScene(const std::string& filePath);
		virtual const bool getShouldUseSeparateTarget() { return true; }
	};
}