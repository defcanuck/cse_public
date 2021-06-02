#pragma once

#include "ClassDef.h"

#include "os/InputManager.h"

#include "scene/behaviors/Behavior.h"
#include "scene/SceneData.h"

#include "ui/UIStack.h"
#include "gfx/PostProcess.h"
#include "global/SerializableHandle.h"
#include "scripting/LuaCallback.h"
#include "global/BitMask.h"
#include "gfx/DisplayList.h"
#include "gfx/RenderTarget.h"
#include "ecs/Entity.h"
#include "animation/Animator.h"

#include <unordered_map>

namespace cs
{

	class Scene;

	typedef void(*sceneInitFunc)(Scene*);
	typedef void(*sceneUpdateFunc)(Scene*, float32);
	
	extern const std::vector<ClearMode> kDefaultClearTextureParams;

	enum SceneRender
	{
		SceneRenderNone = -1,
		SceneRenderDebug,
		SceneRenderDepth,
		SceneRenderPhysics,
		//...
		SceneRenderMAX
	};
	extern const char* kSceneRenderFlag[];

	CLASS_DEFINITION_REFLECT(SceneParams)
	public:
    
		SceneParams()
			: name("")
			, initFunc(nullptr)
			, updateFunc(nullptr)
			, preDrawFunc(nullptr)
			, postDrawFunc(nullptr)
			, viewport(RectI())
			, clearColor(ColorF(0.0f, 0.0f, 0.0f, 0.0f))
			, clearMode(kDefaultClearTextureParams)
			, rttChannels(TextureNone)
			, rttDepth(DepthComponent32)
			, rttName("")
			, forceCreate(false)
			, renderOrder(-1)
            , systemsEnabled(ECSContext::kDefaultSystems)
		{ }

		std::string name;
		sceneInitFunc initFunc;
		sceneUpdateFunc updateFunc;
		DisplayListPass::displayCallbackFunc preDrawFunc;
		DisplayListPass::displayCallbackFunc postDrawFunc;
	
		RectI viewport;
		ColorF clearColor;
		std::vector<ClearMode> clearMode;

        PostProcessParamsMap preProcesses;
		PostProcessParamsMap postProcesses;

		// Render Texture options
		std::string rttName;
		TextureChannels rttChannels;
		Dimensions rttDimm;
		DepthComponent rttDepth;

		bool forceCreate;
		int renderOrder;
    
        ECSSystemMask systemsEnabled;
	
	};

	CLASS_DEFINITION_REFLECT(CopyTexture)
	public:
		RenderTexturePtr source;
		RenderTexturePtr destination;
		ShaderHandlePtr shader;
	};

	class Behavior;

	typedef BitMask<SceneRender, SceneRenderMAX> SceneRenderMask;

	CLASS_DEFINITION(SceneTimer)
	public:
		SceneTimer(const std::string& n, float32 t, LuaCallbackPtr& cb)
			: name(n)
			, maxTime(t)
			, curTime(0.0f)
			, callback(cb)
		{ }

		bool update(float32 dt);

		float32 getPercent() 
		{
			return this->curTime / std::max<float32>(0.00001, this->maxTime);
		}

		std::string name;
		float32 maxTime;
		float32 curTime;
		LuaCallbackPtr callback;

	};

	CLASS_DEFINITION_REFLECT(Scene)
	
	protected:

		struct SceneLock { int dummy; };
        static SceneLock kLock;

	public:

		typedef std::function<void(Scene*)> SceneInitFunc;
		typedef std::function<void(Scene*, float32)> SceneUpdateFunc;
		typedef std::function<bool(const RectI&)> BindMainTargetCallback;

		Scene(SceneLock& lock, SceneParams& params);
		virtual ~Scene();

		virtual void onNew();
		virtual void onPreLoad();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		const std::string& getName() const { return this->name; }
		bool getIsActive() const;
		void setIsActive(bool a);

        bool getIsAnimating() const { return this->animating; }
        void setIsAnimating(bool b) { this->animating = b; }

		bool getIsListening() const;
		void setIsListening(bool l);

		virtual void draw();
		virtual void process(float32 dt);
		virtual void reset();

		void setup(LuaStatePtr& luaState);

        void addPreProcess(PostProcessPtr& preptr);
		void addPostProcess(PostProcessPtr& postptr);

		CameraPtr getCamera() { return camera; }
		SceneDataPtr getSceneData() { return this->data; }

		void setInitCallback(SceneInitFunc func) { this->initFunc = func; }
		void setUpdateCallback(SceneUpdateFunc func) { this->updateFunc = func; }
		void setPreDrawCallback(DisplayListPass::displayCallbackFunc func) { this->preDrawFunc = func; }
		void setPostDrawCallback(DisplayListPass::displayCallbackFunc func) { this->postDrawFunc = func; }

		void clearBehaviorStates();
        void releaseTargets();

		void setECSContext();

		void setClearColor(const ColorF& cc) { this->clearColor = cc; }
		void setTint(const ColorF& c);

		template <class T>
		void addBehavior(std::shared_ptr<T>& behavior, size_t priority = 0)
		{
			std::shared_ptr<Behavior> behavior_base = std::static_pointer_cast<Behavior>(behavior);
			this->addBehaviorImpl(behavior_base, priority);
		}

		void setScriptUpdate(LuaCallbackPtr& luaUpdateFunc);
		void setScriptDraw(LuaCallbackPtr& luaDrawFunc);

		void clearScriptUpdate() { this->onScriptUpdate = nullptr; }
		void clearScriptDraw() { this->onScriptDraw = nullptr; }

		SceneRenderMask renderMask;

		static ColorB getSelectableColor(SelectableVolumeType type);

		void setSceneSpeed(float32 speed);
		float32 getSceneSpeed() const { return this->speedAdjustment; }

		void setAnimSpeed(float32 speed);
		float32 getAnimSpeed() const { return this->animAdjustment; }

		void animateSceneSpeed(float32 desiredSpeed, float32 blendTime, float32 smooth);

		int getRenderOrder() const { return this->renderOrder; }

		void addSceneTimer(SceneTimerPtr& ptr) { this->timers.push_back(ptr); }

		TextureHandlePtr getRenderTexture();
		RenderTexturePtr getRenderTarget();

		void addCopyPass(const std::string& name, TextureHandlePtr& src, RenderTexturePtr& dst, ShaderHandlePtr& shader, bool removeAfterUse);
		void removeCopyyPass(const std::string& name);

		PostProcessPtr getFinalResolvePass() { return this->resolveToFront; }
		const RectI& getFinalResolveViewport() { return this->resolveViewport; }

		BindMainTargetCallback mainTargetCallback;

	protected:

		Scene() { }

		void addBehaviorImpl(BehaviorPtr& behavior, size_t priority);
		void drawDebugImpl();

		friend class InputManager;
		friend class SceneManager;

		std::string name;
		bool active;
		bool listening;
        bool animating;

		PostProcessList preProcess;
		PostProcessList postProcesses;
		PostProcessPtr resolveToFront;
		RectI resolveViewport;

		struct BehaviorWrapper
		{
			BehaviorWrapper(BehaviorPtr ptr, size_t pri = 0)
				: behavior(ptr)
				, priority(pri)
			{ }

			BehaviorWrapper(const BehaviorWrapper& rhs)
				: behavior(rhs.behavior)
				, priority(rhs.priority)
			{ }

			void operator=(const BehaviorWrapper& rhs)
			{
				this->behavior = rhs.behavior;
				this->priority = rhs.priority;
			}

			BehaviorPtr behavior;
			size_t priority;
		};

		typedef std::vector<BehaviorWrapper> BehaviorList;
		BehaviorList behaviors;

		bool onCursor(ClickInput input, ClickParams& params);
		bool onKey(uint32 key, uint32 flags, bool down);
		
		SceneInitFunc initFunc;
		SceneUpdateFunc updateFunc;
		
		DisplayListPass::displayCallbackFunc preDrawFunc;
		DisplayListPass::displayCallbackFunc postDrawFunc;

		friend class Behavior;

		SceneDataPtr data;
		CameraPtr camera;
		ECSSystemMask systemsEnabled;

		ColorF clearColor;
		ColorF tint;
		std::vector<ClearMode> clearModes;

		LuaCallbackPtr onScriptUpdate;
		LuaCallbackPtr onScriptDraw;
		float32 animTime;

		float32 animAdjustment;
		float32 speedAdjustment;
		AnimationInstancePtr speedAnimation;

		RenderTextureSharedData rtt;
		int32 renderOrder;

		typedef std::vector<SceneTimerPtr> SceneTimers;
		SceneTimers timers;

	};

#if defined(CS_WINDOWS)
	template <>
	std::string SerializableHandle<Scene>::getExtension()  { return "scene"; }

	template <>
	std::string SerializableHandle<Scene>::getDescription()  { return "CSE Scene"; }
#else
	template <>
	std::string SerializableHandle<Scene>::getExtension();

	template <>
	std::string SerializableHandle<Scene>::getDescription();
#endif
}
