#pragma once

#include "ClassDef.h"
#include "math/Rect.h"
#include "ui/UIStack.h"
#include "scene/Scene.h"
#include "gfx/Renderer.h"

#include "scripting/LuaCallback.h"

#include "global/SerializableHandle.h"

namespace cs
{
	enum ContextTimer
	{
		ContextTimerSceneProcess,
		ContextTimerSceneRender,
		ContextTimerUIProcess,
		ContextTimerUIRender,
		ContextSceneBatchTraverse,
		ContextSceneBatchBuffer,
		ContextSplineUpdate,
		//...
		ContextTimerMAX
	};

	extern const char* kContextTimerStr[];

	CLASS_DEFINITION(Context)
	public:

		Context(const std::string& name, const RectI& rect);
		virtual ~Context() { }

		const std::string& getContextName() const { return this->contextName; }

		virtual void init();
		virtual void process(float32 dt);
		virtual void render();

		void setForPostProcessing();

		static void preRender();

		void setUIView(RectI& rect) { this->uiView = rect; }
		RectI getUIView() const { return this->uiView; }

		void setContext();

		virtual const bool getShouldUseSeparateTarget() { return false; }
		
		virtual void onActive();
		virtual void onInactive();
		virtual void onUpdateUI() { }

		virtual void play() { }
		virtual void stop() { }
		virtual void reset() { }

		virtual void refresh();
		void clearMainScene();
		void clearScenes();

		void startScript(LuaStateParams& params);

		virtual void populateFileExtensions(StringList& ext) { }
		virtual void populateFileDescriptions(StringList& desc) { }

		virtual void newScene() { }
		virtual bool loadScene(const std::string& filePath) { assert(false);  return false; }
		virtual bool saveScene() { return false; }
		virtual bool saveSceneAs(const std::string& filePath) { assert(false);  return false; }

		float32 getAspectRatio() const;
		float32 getContentScale() const;

		ScenePtr addScene(SceneParamsPtr& params);
		bool removeScene(const std::string& name, bool unloadScene = true);
		bool setMainScene(const std::string& name);
		int32 getNumActiveScenes() { return (int32) this->sortedScenes.size(); }

		virtual bool getIsActiveOnStart() const { return false; }

		bool getShouldSave() const { return this->needsSave; }

		virtual bool onCursor(ClickInput input, ClickParams& params);
		virtual bool onKey(uint32 key, uint32 flags, bool down) { return false; }

		UIStackPtr& getUI() { return this->ui; }
		ScenePtr& getScene() { return this->mainScene; }
    
        void resetUI();

		LuaCallbackPtr onScriptUpdate;

		LuaStatePtr& getLuaState() { return this->luaState; }

		static uint32 updateTimes[ContextTimerMAX];

		void setUseRenderTarget(bool use);
		void setFinalResolveShader(const std::string& name);

	protected:

		bool onBindTarget(const RectI& viewport);
		void updateAccelerometer(float32 dt);
		void addSceneInternal(const std::string& name, SerializableHandle<Scene>& scene);
		void updateSceneList();
		void refreshMainScene();

		ScenePtr mainScene;

		typedef std::unordered_map<std::string, SerializableHandle<Scene>> SceneMap;
		SceneMap scenes;
		typedef std::vector<ScenePtr> SortedSceneList;
		SortedSceneList sortedScenes;

		std::string contextName;
		bool needsSave;
		
		RectI uiView;
		RectI sceneView;
		UIStackPtr ui;

		std::string luaPath;
		LuaStatePtr luaState;

		static UniformPtr accelerometer_x;
		static UniformPtr accelerometer_y;
		static UniformPtr accelerometer_z;

		bool useRenderTarget;
		bool isCapturingInTarget;
		RenderTargetCapture capture;

		RenderTargetCapture stencil;
	};
}
