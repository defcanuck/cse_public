#pragma once

#include "ClassDef.h"
#include "global/Values.h"

#include "scripting/LuaState.h"

#include "game/Context.h"
#include "ui/UIStack.h"

namespace cs
{

	CLASS_DEFINITION(GameInterface)
	public:

		GameInterface();
		virtual ~GameInterface();

		virtual std::string getTitle() const { return "unknown"; }

		virtual bool init(LuaBindInitFunctions& luaFuncs);
		virtual void process(float32 dt) { assert(false); }
		virtual void render() { assert(false); }

		virtual void onStart() { assert(false); }
		virtual void onEnd() { assert(false); }

		void setContext(ContextPtr& cxt);

		ContextPtr& getContext();
		UIStackPtr& getUI();
		ScenePtr& getScene();

		const RectI& getResolution() { return this->gameResolution; }
		void setGameResolution(const RectI& gr) { this->gameResolution = gr; }

		static GameInterfacePtr singleton;
		static GameInterfacePtr getInstance();

		bool loadScene(const std::string& fileName);
		
	protected:

		ContextPtr context;
		RectI gameResolution;

		
	};
}