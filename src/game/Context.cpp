#include "PCH.h"

#include "game/Context.h"

#include "gfx/RenderInterface.h"
#include "scene/Camera.h"
#include "global/Timer.h"
#include "scene/SceneManager.h"
#include "fx/ParticleHeap.h"
#include "scripting/ScriptNotification.h"
#include "physics/PhysicsContact.h"

#if defined(CS_IOS)
    #include "Platform_iOS.h"
#endif

namespace cs
{

	const std::vector<ClearMode> kFrameBufferClearParams = { ClearColor, ClearDepth };
	const ColorF kFrameBufferClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	uint32 Context::updateTimes[ContextTimerMAX];

	UniformPtr Context::accelerometer_x;
	UniformPtr Context::accelerometer_y;
	UniformPtr Context::accelerometer_z;

	const char* kContextTimerStr[] =
	{
		"Scene Process",
		"Scene Render",
		"UI Process",
		"UI Render",
		"Batch Traverse",
		"Batch Buffer Copy",
		"Spline Update"
	};

	Context::Context(const std::string& name, const RectI& rect)
		: uiView(rect)
		, needsSave(false)
		, onScriptUpdate(nullptr)
		, useRenderTarget(false)
		, isCapturingInTarget(false)
	{
		this->contextName = name;
		this->ui = CREATE_CLASS(UIStack);
		this->ui->setSize(this->uiView);

		this->sceneView = this->uiView;
        
        float32 content_scale = RenderInterface::getInstance()->getContentScale();
		this->sceneView.size.w = int32(float32(this->sceneView.size.w) / content_scale);
		this->sceneView.size.h = int32(float32(this->sceneView.size.h) / content_scale);
        
		if (!Context::accelerometer_x) Context::accelerometer_x = SharedUniform::getInstance().getUniform("accelerometer_x");
		if (!Context::accelerometer_y) Context::accelerometer_y = SharedUniform::getInstance().getUniform("accelerometer_y");
		if (!Context::accelerometer_z) Context::accelerometer_z = SharedUniform::getInstance().getUniform("accelerometer_z");
	}

	void Context::setForPostProcessing()
	{
		this->setUseRenderTarget(true);
	}

	float32 Context::getAspectRatio() const
	{
		return this->uiView.size.h / (float32) this->uiView.size.w;
	}

	float32 Context::getContentScale() const
	{
		return RenderInterface::getInstance()->getContentScale();
	}

	void Context::init()
	{

	}

	void Context::preRender()
	{
		TextureAtlasManager::getInstance()->updateAtlases();
		FontManager::getInstance()->updateFonts();
	}

	void Context::updateAccelerometer(float32 dt)
	{
        
        assert(Context::accelerometer_x.get());
        assert(Context::accelerometer_y.get());
        
#if defined(CS_IOS)

        
        float32 accel_x = 0.0f;
        float32 accel_y = 0.0f;
        
        GetBlendedAccelerometerData(&accel_x, &accel_y, 0);
        
        Context::accelerometer_x->setValue(accel_x);
        Context::accelerometer_y->setValue(accel_y);
#else
        
		static float32 accelFake = 0.0;
		accelFake += dt;

		Context::accelerometer_x->setValue(cos(accelFake));
		Context::accelerometer_y->setValue(sin(accelFake));
#endif
	}
	
	void Context::process(float32 dt)
	{

		ParticleHeap::resetStats();
		this->updateAccelerometer(dt);

		ScopedTimer timer(&Context::updateTimes[ContextTimerSceneProcess]);
		SortedSceneList scenesToProcess = this->sortedScenes;

		for (auto& scene : scenesToProcess)
		{
			if (scene.get()) // && scene->getIsActive())
			{
				scene->setECSContext();
				scene->process(dt);
			}
		}
        
        ScriptNotification::getInstance()->flushNotifications();
        
		if (this->ui)
		{
			ScopedTimer timer(&Context::updateTimes[ContextTimerUIProcess]);
			this->ui->process(dt, this->uiView);
		}

		if (this->onScriptUpdate)
		{
			LuaCallback* scriptUpdateCallback = this->onScriptUpdate.get();
			(*scriptUpdateCallback)(dt);
		}

		if (this->capture.resolveToFront.get() && this->capture.resolveToFront->getParams().isDynamic)
		{
			this->capture.resolveToFront->update();
		}
	}

	void Context::setContext()
	{
		if (this->mainScene.get())
		{
			this->mainScene->setECSContext();
		}
	}

	bool Context::onBindTarget(const RectI& viewport)
	{
		renderer::pushRenderTarget("main_scene", this->capture, viewport);
		this->isCapturingInTarget = true;
		return true;
	}

	void Context::setUseRenderTarget(bool use)
	{
		if (this->useRenderTarget != use)
		{
			this->useRenderTarget = use;
			this->refreshMainScene();
		}
	}

	void Context::refreshMainScene()
	{
		if (this->useRenderTarget)
		{
			this->mainScene->mainTargetCallback = std::bind(&Context::onBindTarget, this, std::placeholders::_1);
		}
	}

	void Context::render()
	{
		// Clear the framebuffer
		RenderInterface* render_interface = RenderInterface::getInstance();
		render_interface->setClearColor(kFrameBufferClearColor);
		render_interface->clear(kFrameBufferClearParams);

		ScopedTimer timer(&Context::updateTimes[ContextTimerSceneRender]);
		for (auto& scene : this->sortedScenes)
		{
			if (scene.get()) //&& scene->getIsActive())
			{
				CameraPtr camera = scene->getCamera();
				if (scene.get() == this->mainScene.get())
				{
					camera->setViewport(this->sceneView);
					RenderInterface::getInstance()->setViewport(this->sceneView);
				}
				
				scene->draw();
			}
		}

		cs::UniformPtr color = SharedUniform::getInstance().getUniform("color");
		color->setValue(toVec4(ColorF::White));

		if (this->ui)
		{
			ScopedTimer timer(&Context::updateTimes[ContextTimerUIRender]);
			RenderInterface::getInstance()->setViewport(this->uiView);
			this->ui->draw(this->uiView, UIBatchPassMain);
		}

		if (this->useRenderTarget && this->isCapturingInTarget)
		{
			renderer::popRenderTarget(this->capture, this->uiView);
			this->isCapturingInTarget = false;

			if (this->ui && this->ui->isPassSet(UIBatchPassStencil))
			{
				renderer::pushRenderTarget("stencil", this->stencil, this->uiView, RenderTargetTypeCopyBuffer2);
				this->ui->draw(this->uiView, UIBatchPassStencil);
				renderer::popRenderTarget(this->capture, this->uiView);
			}
			
			// resolve the whole thing to the main pass
			RenderInterface::getInstance()->setDefaultFrameBuffer();
			this->capture.resolveToFront->draw(this->uiView);
		}
	}

	void Context::onActive()
	{
		if (!this->mainScene.get())
			return;

		this->mainScene->setIsListening(true);
		this->mainScene->setIsActive(this->getIsActiveOnStart());
		this->mainScene->setECSContext();

		InputManager::getInstance()->addClickListener(this, &Context::onCursor, 200, "Context");
	}

	void Context::onInactive()
	{
		if (!this->mainScene.get())
			return;

		this->mainScene->setIsListening(false);
		this->mainScene->setIsActive(this->getIsActiveOnStart());

		InputManager::getInstance()->removeClickListener(this, &Context::onCursor);
	}

	void Context::refresh()
	{
		this->getUI()->clearDocuments();
	}

	void Context::startScript(LuaStateParams& params)
	{
        if (!this->mainScene.get())
        {
            return;
        }
		
		{
			SceneDataPtr sceneData;
			ECSContextPtr ecsContext;

			PhysicsContact::gIgnoreScriptEvents = true;
			if (this->mainScene)
			{
				sceneData = this->mainScene->getSceneData();
				ecsContext = sceneData->getContext();
			}

			this->luaState = nullptr;

			PhysicsContact::gIgnoreScriptEvents = false;

			this->mainScene->setECSContext();
			if (sceneData)
			{
				sceneData->empty();
			}
			
		}

		this->luaPath = params.path;
		this->luaState = CREATE_CLASS(LuaState, this->luaPath, LuaState::kLuaGlobals, params);

		bool ret = this->luaState->loadFile(params.entryPoint, true);
		if (ret)
			this->luaState->run();
	}
    
    void Context::resetUI()
    {
        this->ui->clearActionElement();
    }

	bool Context::onCursor(ClickInput input, ClickParams& params)
	{
		this->mainScene->setECSContext();

		CameraPtr camera = this->mainScene->getCamera();
		const RectI& view = camera->getViewport();

		float32 spanW = float32(view.size.w - view.pos.w);
		float32 spanH = float32(view.size.h - view.pos.y);
		if (params.position.x < view.pos.x || params.position.x >(view.pos.x + view.size.w) ||
			params.position.y < view.pos.y || params.position.y >(view.pos.y + view.size.h))
		{
			return false;
		}

		if (this->ui)
		{
			spanW = float32(this->uiView.size.w - this->uiView.pos.w);
			spanH = float32(this->uiView.size.h - this->uiView.pos.y);

			vec2 position = params.position * vec2(spanW, spanH);
			vec2 lastPosition = params.lastPosition * vec2(spanW, spanH);

			ClickParams uiParams = params;
			uiParams.position = position;
			uiParams.lastPosition = position;
			uiParams.screen_dimm = vec2(this->uiView.size.w, this->uiView.size.h);
			return this->ui->onCursor(input, uiParams);
		}

		return false;
	}

	void Context::clearMainScene()
	{
		if (this->mainScene.get())
			this->mainScene->clearBehaviorStates();

		//this->scene;
	}

	void Context::clearScenes()
	{
		this->scenes.clear();
		this->mainScene.reset();
	}

	void Context::addSceneInternal(const std::string& name, SerializableHandle<Scene>& scene)
	{
		assert(scene.get());
		if (this->scenes.find(name) != this->scenes.end())
		{
			log::info("Ignoring duplicate scene add for ", name);
			return;
		}
		this->scenes[name] = scene;
		this->updateSceneList();
	}

	void Context::updateSceneList()
	{

		struct
		{
			bool operator()(const ScenePtr& a, const ScenePtr& b)
			{
				return a->getRenderOrder() > b->getRenderOrder();
			}
		} RenderOrderSort;

		this->sortedScenes.clear();
		std::for_each(this->scenes.begin(), this->scenes.end(), [&](SceneMap::value_type& value)
		{
			ScenePtr& scenePtr = value.second.get();
			this->sortedScenes.push_back(scenePtr);
		});
		std::sort(this->sortedScenes.begin(), this->sortedScenes.end(), RenderOrderSort);
	}

	ScenePtr Context::addScene(SceneParamsPtr& params)
	{
		// Remove any duplicate scene that may have existed here
		if (params->forceCreate)
		{
			SceneManager::getInstance()->removeScene(params->name);
		}

		log::info("Attempting to add scene ", params->name);
		ScenePtr scene = SceneManager::getInstance()->createScene<Scene>(params->name, *(params.get()));
		if (scene.get())
		{
			SerializableHandle<Scene> handle(scene);
			this->addSceneInternal(params->name, handle);
			if (scene->getRenderTexture().get())
			{
				CameraPtr camera = scene->getCamera();
				RectI viewDimm(0, 0, params->rttDimm.w, params->rttDimm.h);
				camera->setViewport(viewDimm);
			}
		}
		else
		{
			log::error("Error creating scene ", params->name);
		}
		return scene;
	}

	bool Context::removeScene(const std::string& name, bool unloadScene)
	{
		
		SceneMap::iterator it = this->scenes.find(name);
		if (it != this->scenes.end())
		{
			this->scenes.erase(it);
			this->updateSceneList();

			if (unloadScene)
			{
				ScenePtr scene = SceneManager::getInstance()->getScene(name);
				if (scene.get())
				{
					scene->getSceneData()->empty();
                    scene->releaseTargets();
				}
			}
			return SceneManager::getInstance()->removeScene(name);
		}

		return false;
	}

	bool Context::setMainScene(const std::string& name)
	{
		SceneMap::iterator it = this->scenes.find(name);
		if (it != this->scenes.end())
		{
			SerializableHandle<Scene>& sceneHandle = it->second;
			this->mainScene = sceneHandle.get();

			CameraPtr camera = this->mainScene->getCamera();
			camera->setViewport(this->sceneView);
			
			this->refreshMainScene();

			return true;
		}

		log::error("No scene found for name ", name);
		return false;

	}

	void Context::setFinalResolveShader(const std::string& name)
	{
		if (this->capture.resolveToFront)
		{
			this->capture.resolveToFront->setShader(name);
		}
	}
}
