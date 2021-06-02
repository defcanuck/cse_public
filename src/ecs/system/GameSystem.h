#pragma once

#include "ecs/system/BaseSystem.h"
#include "ecs/comp/GameComponent.h"

#include "scene/Actor.h"
namespace cs
{

	class GameSystem : public ECSContextSystemBase<GameSystem, ECSGame>, public BaseSystem
	{

	public:
		GameSystem(ECSContext* cxt);
		virtual ~GameSystem();

		virtual void processImpl(SystemUpdateParams* params);

		void testPoint(const vec3& pos);
		
		void setPlayer(EntityPtr& actor, LuaCallbackPtr enterCollision, LuaCallbackPtr exitCollision);
		void clearPlayer();

		virtual void reset();

	private:

		static bool testInternal(const vec3& pos, const CollisionComponentPtr& comp);

		EntityPtr player;
		CollisionComponentPtr lastComponent;

		LuaCallbackPtr onEnterCollision;
		LuaCallbackPtr onExitCollision;
		
	};
}