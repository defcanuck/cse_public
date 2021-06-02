#include "PCH.h"

#include "scripting/LuaBindings.h"
#include "scripting/LuaState.h"

#include "scripting/LuaMacro.h"

#include "scripting/LuaBindMath.h"
#include "scripting/LuaBindGame.h"
#include "scripting/LuaBindUI.h"
#include "scripting/LuaBindGfx.h"
#include "scripting/LuaBindOS.h"
#include "scripting/LuaBindGlobal.h"
#include "scripting/LuaBindECS.h"
#include "scripting/LuaBindScene.h"
#include "scripting/LuaBindAnimation.h"
#include "scripting/LuaBindPhysics.h"
#include "scripting/LuaBindFX.h"
#include "scripting/LuaBindGeom.h"
#include "scripting/LuaBindFont.h"
#include "scripting/LuaBindAudio.h"

#include "os/LogManager.h"
#include "global/ResourceFactory.h"

#if defined(CS_IOS)
    #include "Platform_iOS.h"
#endif

namespace cs
{
	static void print_lua(const std::string& str)
	{
		std::ostringstream oss;
		oss << str;
		LogManager::getInstance()->print(LogLua, oss);
	}

	static void breakpoint()
	{
		log::info("break!");
	}

	static bool getiOS()
	{
#if defined(CS_IOS)
		return true;
#else
		return false;
#endif
	}

    static bool getiOS_Metal()
    {
#if defined(CS_IOS) && defined(CS_METAL)
        return true;
#else
        return false;
#endif
    }

	extern "C" int init(lua_State* state);
	
	void initBindings(lua_State* state)
	{
		init(state);
	}

	void initGlobals(lua_State* state, LuaStateParams* params)
	{
		using namespace luabind;

		object glob = globals(state);
		glob["kIsEditor"] = ((*params).isEditor) ? 1 : 0;
#if defined(CS_IOS)
		glob["kiOS"] = true;
        
#endif

	}

	extern "C" int init(lua_State* state)
	{

		using namespace luabind;

		open(state);
		module(state, "cs")
		[
			def("print", &print_lua),
			def("breakpoint", &breakpoint),
			def("iOS", &getiOS),
            def("iOS_Metal", &getiOS_Metal)
		];

		struct LuaResourceFactory
		{
			static bool load(const std::string& fileName)
			{
				return ResourceFactory::loadFileByExtension(fileName).get() != nullptr;
			}

			static bool loadResource(const std::string& fileName)
			{
				return ResourceFactory::getInstance()->loadResource<SceneReference>(fileName).get() != nullptr;
			}
		};

		module(state, "ResourceFactory")
		[
			def("load", &LuaResourceFactory::load),
			def("beginScope", &ResourceFactory::beginScope),
			def("endScope", &ResourceFactory::endScope),
			def("preloadFromFile", &ResourceFactory::preloadFromFile),
			def("loadResource", &LuaResourceFactory::loadResource)
		];
        
#if defined(CS_IOS)
        struct Lua_iOS
        {
            static std::string getBundleIdentifier()
            {
                return std::string(GetBundleIdentifier());
            }
            static bool isGameCenter()
            {
                return IsGameCenter();
            }
            static std::string getGameCenterUsername()
            {
                const char* userName = GetGameCenterName();
                if (userName)
                {
                    return std::string(userName);
                }
                return "";
            }
            static void triggerHaptic(int level)
            {
                GenerateHapticEvent(level);
            }
            
            static vec3 getAccelerometerData()
            {
                float x = 0.0f, y = 0.0f, z = 0.0f;
                GetAccelerometerData(&x, &y, &z);
                return vec3(x, y, z);
            }
            
            static vec3 getBlendedAccelerometerData()
            {
                float x = 0.0f, y = 0.0f, z = 0.0f;
                GetBlendedAccelerometerData(&x, &y, &z);
                return vec3(x, y, z);
            }
            
            static void startInAppPurchaseRequest()
            {
                RequestInAppPurchases();
            }
            
            static void postAppCenterEvent(const std::string& event)
            {
                PostAppCenterEvent(event.c_str());
            }
        };
        
        module(state, "iOS")
        [
            def("getBundleIdentifier", &Lua_iOS::getBundleIdentifier),
            def("isGameCenter", &Lua_iOS::isGameCenter),
            def("getGameCenterUsername", &Lua_iOS::getGameCenterUsername),
            def("triggerHaptic", &Lua_iOS::triggerHaptic),
            def("getAccelerometerData", &Lua_iOS::getAccelerometerData),
            def("getBlendedAccelerometerData", &Lua_iOS::getBlendedAccelerometerData),
            def("startInAppPurchaseRequest", &Lua_iOS::startInAppPurchaseRequest),
            def("postEvent", &Lua_iOS::postAppCenterEvent)
        ];
        
        struct Lua_TapJoy
        {
            static void requestContent(const std::string& type) { TapJoyRequestContent(type.c_str()); }
            static void preload(const std::string& type) { TapJoyPreload(type.c_str()); }
            static bool show(const std::string& type) { return TapJoyShow(type.c_str()); }
        };
        
        module(state, "TapJoy")
        [
            def("requestContent", &Lua_TapJoy::requestContent),
            def("preload", &Lua_TapJoy::preload),
            def("show", &Lua_TapJoy::show)
        ];
#endif

		module(state)
		[
			// GLOBAL
			BIND_LUA_CLASS(ClickInput),
			BIND_LUA_CLASS(TouchState),
			BIND_LUA_CLASS(LuaCallback),
			BIND_LUA_CLASS(Event),
			BIND_LUA_CLASS(LoadFlags),
			BIND_LUA_CLASS(LoadFlagMask),
			BIND_LUA_CLASS(PropertySetHandle),
			BIND_LUA_CLASS(PropertySetResource),
            BIND_LUA_CLASS(ScriptNotification),
			BIND_LUA_CLASS(InputManager),

			// MATH
			BIND_LUA_CLASS(vec2),
			BIND_LUA_CLASS(vec3),
			BIND_LUA_CLASS(vec4),
			BIND_LUA_CLASS(quat),
			BIND_LUA_CLASS(PointF),
			BIND_LUA_CLASS(PointI),
			BIND_LUA_CLASS(RectF),
			BIND_LUA_CLASS(RectI),
			BIND_LUA_CLASS(Ray),
			BIND_LUA_CLASS(Plane),

			// GAME
			BIND_LUA_CLASS(GameInterface),
			BIND_LUA_CLASS(Context),

			// GEOM
			BIND_LUA_CLASS(Volume),
			BIND_LUA_CLASS(QuadVolume),

			// OS
			BIND_LUA_CLASS(FileSystemNode),
			BIND_LUA_CLASS(FileDataType),
			BIND_LUA_CLASS(FileSystemBase),
			BIND_LUA_CLASS(FileSystemEntry),
			BIND_LUA_CLASS(FileSystemDirectory),
			BIND_LUA_CLASS(FileManager),
#if defined(CS_WINDOWS)
			BIND_LUA_CLASS(FileManager_Windows),
#endif
			BIND_LUA_CLASS(LogManager),
            BIND_LUA_CLASS(Leaderboard),
            BIND_LUA_CLASS(Analytics),

			// UI
			BIND_LUA_CLASS(SpanMeasure),
			BIND_LUA_CLASS(HAlign),
			BIND_LUA_CLASS(VAlign),
			BIND_LUA_CLASS(UIClickBehavior),
			BIND_LUA_CLASS(UIHoverBehavior),
			BIND_LUA_CLASS(UIClickAnimationBehavior),
			BIND_LUA_CLASS(UIElement),
			BIND_LUA_CLASS(UITextElement),
			BIND_LUA_CLASS(UIDocument),
			BIND_LUA_CLASS(UIStack),
			BIND_LUA_CLASS(UIDecorator),
			BIND_LUA_CLASS(UIOutlineDecorator),
			BIND_LUA_CLASS(UIComponentDecorator),
			BIND_LUA_CLASS(UIComponentDecoratorType),
			BIND_LUA_CLASS(UIComponentDecoratorQuadrant),
			BIND_LUA_CLASS(UIAnimationType),
			BIND_LUA_CLASS(UITextAnimationType),
			BIND_LUA_CLASS(UIAnimationCreator),
			BIND_LUA_CLASS(UISlider),
			BIND_LUA_CLASS(UIAnimationPlayer),
			BIND_LUA_CLASS(UIFlipView),
			BIND_LUA_CLASS(UIElementFlag),
			BIND_LUA_CLASS(UIBatchPass),

			// GFX
            BIND_LUA_CLASS(RenderInterface),
			BIND_LUA_CLASS(Shape),
			BIND_LUA_CLASS(QuadShape),
			BIND_LUA_CLASS(ParallelogramShape),
			BIND_LUA_CLASS(RenderTargetType),
			BIND_LUA_CLASS(ColorFList),
			BIND_LUA_CLASS(ColorF),
			BIND_LUA_CLASS(ColorB),
			BIND_LUA_CLASS(Renderable),
			BIND_LUA_CLASS(BatchRenderable),
			BIND_LUA_CLASS(MeshRenderable),
			BIND_LUA_CLASS(TextureHandle),
			BIND_LUA_CLASS(Texture),
			BIND_LUA_CLASS(TextureStage),
			BIND_LUA_CLASS(RenderableOptions),
			BIND_LUA_CLASS(DrawOptions),
			BIND_LUA_CLASS(DrawOptionsBlend),
			BIND_LUA_CLASS(DrawOptionsDepth),
			BIND_LUA_CLASS(DepthType),
			BIND_LUA_CLASS(BlendType),
			BIND_LUA_CLASS(TrailRenderable),
			BIND_LUA_CLASS(RenderTraversal),
			BIND_LUA_CLASS(RenderTraversalMask),
			BIND_LUA_CLASS(ShaderHandle),
			BIND_LUA_CLASS(TextureChannels),
			BIND_LUA_CLASS(DepthComponent),
			BIND_LUA_CLASS(MeshHandle),
			BIND_LUA_CLASS(Mesh),
			BIND_LUA_CLASS(MeshShape),
			BIND_LUA_CLASS(MeshShapeInstance),
			BIND_LUA_CLASS(MeshMaterialType),
			BIND_LUA_CLASS(SplineRenderable),
			BIND_LUA_CLASS(SharedUniform),
			BIND_LUA_CLASS(RenderTexture),
			BIND_LUA_CLASS(RenderTargetManager),
			BIND_LUA_CLASS(PostProcess),

			// PHYSICS
			BIND_LUA_CLASS(PhysicsLiquidContactData),
			BIND_LUA_CLASS(PhysicsBodyCollision),
			BIND_LUA_CLASS(PhysicsBodyCollisionScript),
			BIND_LUA_CLASS(PhysicsBody),
			BIND_LUA_CLASS(PhysicsShape),
			BIND_LUA_CLASS(PhysicsShapeBox),

			// ECS
            BIND_LUA_CLASS(ECSSystems),
            BIND_LUA_CLASS(ECSSystemMask),
			BIND_LUA_CLASS(SceneNode),
			BIND_LUA_CLASS(Entity),
			BIND_LUA_CLASS(Component),
			BIND_LUA_CLASS(EntitySearchParams),
			BIND_LUA_CLASS(EntitySharedList),

			// DRAWABLE SYSTEM
			BIND_LUA_CLASS(DrawableComponent),
			BIND_LUA_CLASS(DrawableSystem),

			// FONT
			BIND_LUA_CLASS(Font),
			BIND_LUA_CLASS(FontManager),
			
			// PHYSICS SYSTEM
			BIND_LUA_CLASS(PhysicsComponent),
			BIND_LUA_CLASS(PhysicsSystem),
			
			// SCRIPT SYSTEM
			BIND_LUA_CLASS(ScriptInstanceState),
			BIND_LUA_CLASS(ScriptComponent),
			BIND_LUA_CLASS(ScriptComponentInstance),
			BIND_LUA_CLASS(GameSelectableType),

			// ANIMATION SYSTEM
			BIND_LUA_CLASS(AnimationComponent),
			BIND_LUA_CLASS(AnimationScaleType),
			
			// GAME SYSTEM
			BIND_LUA_CLASS(GameComponent),
			BIND_LUA_CLASS(GameComponentList),
			BIND_LUA_CLASS(GameSystem),
			BIND_LUA_CLASS(GameSelectableBehavior),
			BIND_LUA_CLASS(CollisionComponent),
			BIND_LUA_CLASS(CollisionScriptInstance),

			// SCENE
            BIND_LUA_CLASS(HighPrecisionTimer),
			BIND_LUA_CLASS(SceneTimer),
			BIND_LUA_CLASS(Scene),
			BIND_LUA_CLASS(SceneData),
			BIND_LUA_CLASS(SceneCreator),
			BIND_LUA_CLASS(SceneCreateComponent),
			BIND_LUA_CLASS(SceneCreateComponentMask),
			BIND_LUA_CLASS(SceneRender),
			BIND_LUA_CLASS(SceneRenderMask),
			BIND_LUA_CLASS(SceneParams),
			BIND_LUA_CLASS(SceneReferenceCache),

			BIND_LUA_CLASS(Actor),
			BIND_LUA_CLASS(Camera),
			BIND_LUA_CLASS(Light),
			BIND_LUA_CLASS(Behavior),
			BIND_LUA_CLASS(CameraMoveBehavior),
			BIND_LUA_CLASS(ScriptInputBehavior),
			BIND_LUA_CLASS(GameInputBehavior),
			BIND_LUA_CLASS(ReferenceNode),

			// FX
			BIND_LUA_CLASS(ParticleEmitter),
			BIND_LUA_CLASS(ParticleEmitterAnchor),
			BIND_LUA_CLASS(ParticleEmitterScope),

			// Animation
			BIND_LUA_CLASS(AnimationType),
			BIND_LUA_CLASS(AnimationBase),
			BIND_LUA_CLASS(FloatAnimation),
			BIND_LUA_CLASS(Vec2Animation),
			BIND_LUA_CLASS(Vec3Animation),
			BIND_LUA_CLASS(ColorBAnimation),
			BIND_LUA_CLASS(ColorFAnimation),
			BIND_LUA_CLASS(FloatLerpAnimator),
			BIND_LUA_CLASS(Vec2LerpAnimator),
			BIND_LUA_CLASS(Vec3LerpAnimator),
			BIND_LUA_CLASS(ColorBLerpAnimator),
			BIND_LUA_CLASS(ColorFLerpAnimator),
			BIND_LUA_CLASS(AnimatedValue),
			BIND_LUA_CLASS(AnimationSize),
			BIND_LUA_CLASS(AnimationTextureUV),
			BIND_LUA_CLASS(AnimationColor),
			BIND_LUA_CLASS(AnimationMethod),
			BIND_LUA_CLASS(AnimationMethodPath),
			BIND_LUA_CLASS(AnimationMethodValue),
			
			// AUDIO

			BIND_LUA_CLASS(SoundManager),

			// standalone functions
			def("degreesToRadians", &degreesToRadians),
			def("radiansToDegrees", &radiansToDegrees)
			
		];

		return 0;

	}
}
