#include "PCH.h"

#include "scene/Scene.h"
#include "gfx/RenderInterface.h"
#include "gfx/Shader.h"

#include "os/LogManager.h"

#include "ecs/system/DrawableSystem.h"
#include "ecs/system/PhysicsSystem.h"
#include "ecs/system/ScriptSystem.h"
#include "ecs/system/AnimationSystem.h"
#include "ecs/system/ParticleSystem.h"
#include "ecs/system/GameSystem.h"
#include "ecs/system/AudioSystem.h"

#include "gfx/VolumeDraw.h"
#include "serial/RefVariant.h"
#include "serial/JSON.h"
#include "ecs/ECS_Utils.h"

namespace cs
{

	const char* kSceneRenderFlag[] =
	{
		"debug",	 // SceneRenderDebug
		"depth",	 // SceneRenderDepth
	};

    Scene::SceneLock Scene::kLock;
    
	BEGIN_META_CLASS(SceneParams)

	END_META()

	BEGIN_META_CLASS(Scene)
		ADD_MEMBER_PTR(data);
	END_META()

	ColorB Scene::getSelectableColor(SelectableVolumeType type)
	{
		switch (type)
		{
			case SelectableVolumeTypeDraw:
				return ColorB(255, 255, 0, 255);
			case SelectableVolumeTypePhysics:
				return ColorB(255, 64, 64, 255);
			case SelectableVolumeTypeFace:
				return ColorB(128, 0, 255, 255);
			case SelectableVolumeTypeVertex:
				return ColorB(255, 128, 0, 255);
			case SelectableVolumeTypeTriangle:
				return ColorB(64, 32, 255, 255);
			case SelectableVolumeTypeCollision:
				return ColorB(255, 0, 0, 255);
			case SelectableVolumeTypeReference:
				return ColorB(255, 0, 255, 255);
			case SelectableVolumeTypeDirection:
				return ColorB(100, 100, 255, 255);
			case SelectableVolumeTypePositionAnimation:
				return ColorB(64, 255, 32, 255);
			case SelectableVolumeTypeSizeAnimation:
				return ColorB(100, 255, 100, 255);
			case SelectableVolumeTypeLight:
				return ColorB(0, 128, 64, 255);
			default:
				return ColorB::White;
		}
	}

	bool SceneTimer::update(float32 dt)
	{
		this->curTime += dt;
		if (this->curTime >= this->maxTime)
		{
			log::info("ScriptTimer: signaling ", this->name);
			(*this->callback)();
			return true;
		}
		return false;
	}
    
	Scene::Scene(SceneLock& lock, SceneParams& params)
		: name(params.name)
		, active(true)
		, listening(true)
        , animating(true)
		, animTime(0.0f)
		, speedAdjustment(1.0f)
		, animAdjustment(1.0f)
		, renderOrder(params.renderOrder)
		, tint(ColorF::White)
        , systemsEnabled(params.systemsEnabled)
	{
		if (params.initFunc)
		{
			this->initFunc = std::bind(params.initFunc, std::placeholders::_1);
		}

		if (params.updateFunc)
		{
			this->updateFunc = std::bind(params.updateFunc, std::placeholders::_1, std::placeholders::_2);
		}

		this->preDrawFunc = params.preDrawFunc;
		this->postDrawFunc = params.postDrawFunc;

		this->data = CREATE_CLASS(SceneData, this->name, this->systemsEnabled);
        
		this->camera = CREATE_CLASS(Camera);

        this->clearModes = params.clearMode;
		this->clearColor = params.clearColor;

		for (auto it : params.preProcesses)
		{
			PostProcessPtr post = CREATE_CLASS(PostProcess, it.first, it.second);
			
			post->mapInputs(RenderTargetManager::getInstance()->renderTargets);
			post->mapOutputs(RenderTargetManager::getInstance()->renderTargets);

			this->preProcess.push_back(post);
		}
        
        for (auto it : params.postProcesses)
        {
            PostProcessPtr post = CREATE_CLASS(PostProcess, it.first, it.second);
            
            post->mapInputs(RenderTargetManager::getInstance()->renderTargets);
            post->mapOutputs(RenderTargetManager::getInstance()->renderTargets);

            this->postProcesses.push_back(post);
        }
		  
		//bool doFinalResolve = RenderInterface::getInstance()->getContentScale() > 1.0f;
		bool doFinalResolve = true;
		
		if (!this->rtt.target.get() && doFinalResolve && !this->resolveToFront)
		{

			this->resolveViewport = params.viewport;
			
			PostProcessParams resolveToFrontParams;
			// resolveToFrontParams.shader = RenderInterface::kDefaultTextureShader;
			resolveToFrontParams.viewport = this->resolveViewport;
            resolveToFrontParams.inputMap   .push_back(RenderTextureInputMapping(RenderTargetManager::kUseBackBuffer, TextureStageDiffuse));
			resolveToFrontParams.outputMap.targetBuffer = RenderTargetTypeBackBufferFloat;
			resolveToFrontParams.clearModes = { ClearColor, ClearDepth };
			resolveToFrontParams.clearColor = ColorF(1.0f, 1.0f, 0.0f, 1.0f);
			resolveToFrontParams.isDynamic = true;

			TextureHandlePtr sampleTexture = CREATE_CLASS(TextureHandle, RenderTargetManager::getInstance()->getTarget(RenderTargetManager::kUseBackBuffer)->getTexture());
			//TextureHandlePtr sampleTexture = CREATE_CLASS(TextureHandle, "test_texture.png");

			resolveToFrontParams.fillRect = sampleTexture->getUVRect();
			
#if !defined(CS_METAL)
            resolveToFrontParams.fillRect.flipVertical();
#else
            // resolveToFrontParams.fillRect = RectF(0.0f, 1.0f, 1.0f, -1.0f);
#endif
			this->resolveToFront = CREATE_CLASS(PostProcess, "ResolveToFront", resolveToFrontParams);
			this->resolveToFront->mapInputs(RenderTargetManager::getInstance()->renderTargets);
			this->resolveToFront->setTexture(sampleTexture, TextureStageDiffuse);
		}

		if (params.rttName.length() > 0 && params.rttChannels != TextureNone)
		{
			std::string rttName = params.rttName;
			if (rttName.length() <= 0)
			{
				std::stringstream str;
				str << rttName << "_rtt";
				rttName = str.str();
			}

			RenderTargetManager::getInstance()->mapSharedTarget(rttName, params.rttDimm, params.rttChannels, params.rttDepth, this->rtt);

		}
	}

	Scene::~Scene()
	{
		this->clearBehaviorStates();
        this->releaseTargets();
	}
    
    void Scene::releaseTargets()
    {
        if (this->rtt.shared)
        {
            RenderTargetManager::getInstance()->unmapSharedTarget(this->rtt);
        }
    }

	void Scene::onNew()
	{
		if (this->data)
		{
			this->data->setContext();
			this->data->reset();
			this->data = nullptr;
		}

		this->data = CREATE_CLASS(SceneData, this->name);
	}

	void Scene::onPreLoad()
	{
		this->data = nullptr;
	}

	void Scene::setup(LuaStatePtr& luaState)
	{
		this->data->prepareSceneContext(luaState);
		this->data->onPostLoad();
	}

	void Scene::onPostLoad(const LoadFlagMask& flags)
	{
		log::info("Ignoring Scene::onPostLoad");
	}

	void Scene::setTint(const ColorF& c) 
	{ 
		this->tint = c; 
	}

	void Scene::draw()
	{
		this->data->setContext();

		if (this->onScriptDraw)
			(*this->onScriptDraw.get())();

		bool isSubScene = false;
		if (this->rtt.target.get())
		{
			this->rtt.target->bind();
			isSubScene = true;
		}

		UniformPtr anim_time = SharedUniform::getInstance().getUniform("animation_time");
		assert(anim_time);
		anim_time->setValue(this->animTime);

		UniformPtr anim_time_vtx = SharedUniform::getInstance().getUniform("animation_time_vtx");
		assert(anim_time_vtx);
		anim_time_vtx->setValue(this->animTime);

		UniformPtr anim_time_pct = SharedUniform::getInstance().getUniform("animation_pct");
		assert(anim_time_pct);
		anim_time_pct->setValue(this->animTime - float32(int32(this->animTime)));

		this->data->bindLights();

		DisplayList display_list;

		bool callbackToMainPass = (!isSubScene && this->resolveToFront.get());
		
		// Call pre-draw flag (maybe refactored!?)
		if (callbackToMainPass)
		{
			if (this->preDrawFunc)
				this->preDrawFunc(this);
		}

		PostProcessList toEraseList;
		if (this->preProcess.size() > 0)
		{
			for (PostProcessList::iterator it = this->preProcess.begin(); it != this->preProcess.end(); )
			{
				(*it)->resolve(display_list);
				const PostProcessParams& params = (*it)->getParams();
				if (params.removeAfterUse)
				{
					toEraseList.push_back(*it); // keep the pointer valid until the render has flushed
					it = this->preProcess.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		if (!isSubScene)
		{
			DisplayListPassPtr depth_pass = display_list.addPass("Shadow", RenderTraversalShadow);
			depth_pass->traversalMask.set(RenderTraversalShadow);
			depth_pass->target = RenderTargetManager::getInstance()->getTarget(RenderTargetTypeCopyBuffer);
			depth_pass->targetViewport = this->camera->getViewport();
			depth_pass->clearColor = ColorF::Black;
			depth_pass->clearModes = this->clearModes;
            depth_pass->zNear = this->camera->getNear();
            depth_pass->zFar = this->camera->getFar();
			display_list.traversals[RenderTraversalShadow].camera = this->camera;
		}

		display_list.traversals[RenderTraversalMain].camera = this->camera;
        
        std::string passName = this->getName() + "_Main";
		DisplayListPassPtr main_pass = display_list.addPass(passName, RenderTraversalMain);
		main_pass->traversalMask.set(RenderTraversalMain);
		if (isSubScene)
		{
			main_pass->clearColor = this->clearColor;
			main_pass->clearModes = this->clearModes;
			main_pass->target = this->rtt.target;
			main_pass->targetViewport = this->camera->getViewport();
            main_pass->zNear = this->camera->getNear();
            main_pass->zFar = this->camera->getFar();
		}
		else if (this->resolveToFront.get())
		{
			main_pass->clearColor = this->clearColor;
			main_pass->clearModes = this->clearModes;
			main_pass->target = RenderTargetManager::getInstance()->getTarget(RenderTargetManager::kUseBackBuffer);
			main_pass->targetViewport = this->camera->getViewport();
			main_pass->preCallback = this->preDrawFunc;
            main_pass->zNear = this->camera->getNear();
            main_pass->zFar = this->camera->getFar();
		}

        if (this->systemsEnabled.test(ECSDraw))
            DrawableSystem::getInstance()->flush(display_list);
        
        if (this->systemsEnabled.test(ECSParticle))
            ParticleSystem::getInstance()->flush(display_list);

		if (this->preProcess.size() > 0)
		{
			// Post Process the results of the back buffer
			// last post process *must* write out to the front buffer
			for (auto it : this->preProcess)
			{
				it->resolve(display_list);
			}
		}

		DisplayListUtil::DrawParams drawParams;
		drawParams.tint = this->tint;
		drawParams.scene = this;

		// draw the display list!
		display_list.draw(&drawParams);

		bool drawToRenderTarget = false;
		if (this->mainTargetCallback)
		{
			drawToRenderTarget = this->mainTargetCallback(this->resolveViewport);
		}

		if (this->resolveToFront.get())
		{
			if (!drawToRenderTarget)
			{
				RenderInterface::getInstance()->setDefaultFrameBuffer();
			}
			this->resolveToFront->draw(this->resolveViewport);
		}

		if (callbackToMainPass)
		{
			if (this->postDrawFunc)
				this->postDrawFunc(this);
		}

		if (!isSubScene)
		{
			this->drawDebugImpl();
		}

	}

	void Scene::drawDebugImpl()
	{
		if (this->renderMask.test(SceneRenderDebug))
		{
			struct DebugRender
			{
				static void renderDebugOverlayCallback(Entity* entity, void* data)
				{
					SelectableVolumeList volumes;
					entity->getSelectableVolume(volumes);
					CameraPtr* camera = (CameraPtr*) data;

					for (auto it : volumes)
					{
						if (it.type != SelectableVolumeTypePhysics)
							continue;

						VolumeDrawPtr volume_draw = CREATE_CLASS(cs::VolumeDraw, it.volume, Scene::getSelectableColor(it.type));
						Transform offset;
						offset.setPosition(it.offset);
						volume_draw->draw(*camera, offset.concatenate(entity->getWorldTransform()));
					}
				}
			};
			
			Entity::Collection& entities = this->data->getEntities();
			entities.traverse(&DebugRender::renderDebugOverlayCallback, (void*) &this->camera);
		}
		else if (this->renderMask.test(SceneRenderPhysics))
		{
			struct DebugRender
			{
				static void renderPhysicsOverlayCallback(Entity* entity, void* data)
				{
					SelectableVolumeList volumes;
					entity->getSelectableVolume(volumes);
					CameraPtr* camera = (CameraPtr*)data;

					for (auto it : volumes)
					{
						if (it.type != SelectableVolumeTypePhysics)
							continue;

						VolumeDrawPtr volume_draw = CREATE_CLASS(cs::VolumeDraw, it.volume, Scene::getSelectableColor(it.type));
						Transform offset;
						offset.setPosition(it.offset);
						volume_draw->draw(*camera, offset.concatenate(entity->getWorldTransform()));
					}
				}
			};

			Entity::Collection& entities = this->data->getEntities();
			entities.traverse(&DebugRender::renderPhysicsOverlayCallback, (void*)&this->camera);
		}
	}

	void Scene::reset()
	{
		if (this->data)
		{
			this->data->reset(this->active);
		}
	}

    void Scene::addPreProcess(PostProcessPtr& preptr)
    {
        
    }

	void Scene::addPostProcess(PostProcessPtr& postptr)
	{
		this->postProcesses.push_back(postptr);
	}

	void Scene::setSceneSpeed(float32 speed)
	{
		this->speedAdjustment = speed;
	}

	void Scene::setAnimSpeed(float32 speed)
	{
		this->animAdjustment = speed;
	}

	void Scene::animateSceneSpeed(float32 desiredSpeed, float32 blendTime, float32 smooth)
	{
		this->speedAnimation = std::static_pointer_cast<AnimationInstance>(
			CREATE_CLASS(
				AnimationInstanceTyped<float32>,
				LerpAnimator<float32>::createAnimation(
					this->speedAdjustment, 
					desiredSpeed, 
					blendTime, 
					smooth, 
					AnimationTypeNone)
			)
		);
	}

	void Scene::process(float32 dt)
	{
        
        std::string tag;
        tag = this->getName() + "_Process";
        RenderInterface::getInstance()->pushDebugScope(tag);
		SceneTimers::iterator it = this->timers.begin();
		while (it != this->timers.end())
		{
			if ((*it)->update(dt))
			{
				it = this->timers.erase(it);
			}
			else
			{
				++it;
			}
		}

		if (this->speedAnimation.get())
		{
			this->speedAnimation->process(dt);
			if (this->speedAnimation->isActive())
			{
				std::shared_ptr<AnimationInstanceTyped<float32>> anim_ptr =
					std::static_pointer_cast<AnimationInstanceTyped<float32>>(this->speedAnimation);
				this->speedAdjustment = anim_ptr->animation.getValue();
			}
			else
			{
				this->speedAnimation = nullptr;
			}
		}


		this->animTime += dt;

		float32 useDt = dt * this->speedAdjustment;
		float32 animateDt = dt * this->animAdjustment;

		this->data->setContext();
		
		if (this->active)
		{
			if (this->onScriptUpdate)
			{
				LuaCallback* scriptUpdateCallback = this->onScriptUpdate.get();
				(*scriptUpdateCallback)(dt, useDt);
			}
		}

		SystemUpdateParams systemParams;
		systemParams.animationDt = animateDt;
		systemParams.updateDt = useDt;
		systemParams.camera = this->camera;
		systemParams.active = this->active;

        if (this->getIsAnimating())
        {
            if (this->systemsEnabled.test(ECSDraw))
            {
                DrawableSystem::getInstance()->setCullCamera(this->camera);
                DrawableSystem::getInstance()->process(&systemParams);
            }
        }
        
		if (this->active)
		{
			if (this->updateFunc)
				this->updateFunc(this, useDt);
			
            this->setECSContext();
            
			if (this->systemsEnabled.test(ECSAnimation)) AnimationSystem::getInstance()->process(&systemParams);
			if (this->systemsEnabled.test(ECSParticle)) ParticleSystem::getInstance()->process(&systemParams);
			if (this->systemsEnabled.test(ECSScript)) ScriptSystem::getInstance()->process(&systemParams);
			if (this->systemsEnabled.test(ECSPhysics)) PhysicsSystem::getInstance()->process(&systemParams);
			if (this->systemsEnabled.test(ECSGame)) GameSystem::getInstance()->process(&systemParams);
		}

		this->setECSContext();

		if (this->systemsEnabled.test(ECSAudio)) AudioSystem::getInstance()->process(&systemParams);
        
		if (this->resolveToFront.get() && this->resolveToFront->getParams().isDynamic)
		{
			this->resolveToFront->update();
		}

        RenderInterface::getInstance()->popDebugScope();
	}

	bool Scene::getIsActive() const 
	{ 
		return this->active; 
	}

	void Scene::setIsActive(bool a) 
	{
		if (this->active == a)
			return;

		// on deactivating scene, just stop all sound playing
		if (this->active && !a)
		{
			if (this->systemsEnabled.test(ECSAudio)) AudioSystem::getInstance()->stopAll();
		}
		this->active = a;
	}

	void Scene::setScriptUpdate(LuaCallbackPtr& luaUpdateFunc)
	{
		this->onScriptUpdate = luaUpdateFunc;
	}

	void Scene::setScriptDraw(LuaCallbackPtr& luaDrawFunc)
	{
		this->onScriptDraw = luaDrawFunc;
	}

	bool Scene::getIsListening() const
	{ 
		return this->listening; 
	
	}
	void Scene::setIsListening(bool l)
	{ 
		this->listening = l; 

		if (this->listening)
		{
			InputManager::getInstance()->addClickListener(this, &Scene::onCursor, 50, this->getName());
			InputManager::getInstance()->addKeyListener(this, &Scene::onKey, 200, this->getName());
		}
		else {
			InputManager::getInstance()->removeClickListener(this, &Scene::onCursor);
			InputManager::getInstance()->removeKeyListener(this, &Scene::onKey);
		}
	
	}

	bool Scene::onKey(uint32 key, uint32 flags, bool down)
	{
		return false;
	}

	bool Scene::onCursor(ClickInput input, ClickParams& params)
	{
		if (!this->getIsListening())
			return false;

		bool sceneConsume = false;
		const RectI& view = this->camera->getViewport();

		float32 spanW = float32(view.size.w - view.pos.w);
		float32 spanH = float32(view.size.h - view.pos.y);
		if (params.position.x < view.pos.x || params.position.x >(view.pos.x + view.size.w) ||
			params.position.y < view.pos.y || params.position.y >(view.pos.y + view.size.h))
		{
			return false;
		}

		vec2 position = params.position * vec2(spanW, spanH);
		vec2 lastPosition = params.lastPosition * vec2(spanW, spanH);

		BehaviorData data;
		this->camera->getRay(params.position, data.ray);
		data.zoom = params.zoom;
		data.screenPos = position;
		data.lastSceenPos = lastPosition;
		data.input = input;

		for (auto it : this->behaviors)
		{
			BehaviorPtr& behavior = it.behavior;
			if (!behavior->getEnabled())
				continue;

			switch (params.state)
			{
				case TouchDown:
					sceneConsume = behavior->onInputDown(this, data);
					break;
				case TouchUp:
					sceneConsume = behavior->onInputUp(this, data);
					break;
				case TouchMove:
					sceneConsume = behavior->onInputMove(this, data);
					break;
				case TouchPressed:
					sceneConsume = behavior->onInputPressed(this, data);
					break;
				case TouchReleased:
					sceneConsume = behavior->onInputReleased(this, data);
					break;
				case TouchWheel:
					sceneConsume = behavior->onInputWheel(this, data);
					break;

				case TouchSwipeLeft:
				case TouchSwipeRight:
				case TouchSwipeUp:
				case TouchSwipeDown:
					// do nothing
					break;

				default:
					assert(false);
					break;
			}

			if (sceneConsume)
				break;
		}

		return sceneConsume;

	}

	void Scene::clearBehaviorStates()
	{
		for (auto it : this->behaviors)
			it.behavior->reset();
	}

	void Scene::addBehaviorImpl(BehaviorPtr& behavior, size_t priority)
	{
		struct
		{
			bool operator()(const BehaviorWrapper& a, const BehaviorWrapper& b)
			{
				return a.priority > b.priority;
			}
		} BehaviorWrapperSort;

		BehaviorWrapper wrapper(behavior, priority);
		this->behaviors.push_back(wrapper);
	
		std::sort(this->behaviors.begin(), this->behaviors.end(), BehaviorWrapperSort);

	}

	void Scene::setECSContext()
	{
		if (!this->data)
			return;

		this->data->setContext();
	}


	TextureHandlePtr Scene::getRenderTexture()
	{
		if (!this->rtt.target.get())
		{
			log::warning("No render texture defined for the scene!");
			return RenderInterface::kErrorTexture;
		}
			
		return CREATE_CLASS(TextureHandle, this->rtt.target->getTexture());
	}

	RenderTexturePtr Scene::getRenderTarget()
	{
		return this->rtt.target;
	}

	void Scene::addCopyPass(const std::string& name, TextureHandlePtr& src, RenderTexturePtr& dst, ShaderHandlePtr& shader, bool removeAfterUse)
	{
		std::vector<RenderTextureInputMapping> inputMap;
		Dimensions dimm = dst->getDimensions();

		PostProcessParams params;
		params.viewport = RectI(0, 0, dimm.w, dimm.h);
		params.fillRect = RectF(0.0f, 0.0f, 1.0f, 1.0f);
		params.shader = shader;
		params.removeAfterUse = removeAfterUse;


		PostProcessPtr post = CREATE_CLASS(PostProcess, name, params);
		post->setTexture(src, TextureStageDiffuse);
		post->setOutput(dst);

		this->preProcess.push_back(post);
	}

	void Scene::removeCopyyPass(const std::string& name)
	{

	}

#if !defined(CS_WINDOWS)

    template <>
    std::string SerializableHandle<Scene>::getExtension()
    {
        return "scene";
    }
    
    template <>
    std::string SerializableHandle<Scene>::getDescription()
    {
        return "CSE Scene";
    }
#endif

}
