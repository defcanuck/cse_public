#pragma once

#include "ClassDef.h"

#include "scripting/LuaCallback.h"
#include "ecs/comp/Component.h"
#include "scene/behaviors/Behavior.h"

#include "geom/Volume.h"

namespace cs
{

	enum GameSelectableType
	{
		GameSelectableTypeNone = -1,
		GameSelectableTypeMove,
		GameSelectableTypeRotate,
		GameSelectableTypeMAX
	};

	CLASS_DEFINITION_REFLECT(GameSelectableBehavior)
	public:
		GameSelectableBehavior()
		{ }

		virtual GameSelectableType getType() const { return GameSelectableTypeNone; }

		LuaCallbackPtr onPressed;
		LuaCallbackPtr onMove;
		LuaCallbackPtr onReleased;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(GameSelectableMoveBehavior, GameSelectableBehavior)
	public:
		GameSelectableMoveBehavior() : GameSelectableBehavior() { }

		virtual GameSelectableType getType() const { return GameSelectableTypeMove; }
	
	};

	CLASS_DEFINITION_DERIVED_REFLECT(GameSelectableRotateBehavior, GameSelectableBehavior)
	public:
		GameSelectableRotateBehavior() : GameSelectableBehavior() { }

		virtual GameSelectableType getType() const { return GameSelectableTypeRotate; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(GameComponent, Component)
	public:
		GameComponent();
		virtual ~GameComponent() { }

		void process(float32 dt);

		bool isSelectable() const { return this->behavior.get() != nullptr; }

		GameSelectableBehaviorPtr& getBehavior() { return this->behavior; }
		const VolumePtr& getInputVolume() const { return this->volume; }

		void computeQuadVolume();
		void computeRadiusVolume();



	private:

		GameSelectableBehaviorPtr behavior;
		VolumePtr volume;

	};

	typedef SharedList<GameComponent> GameComponentList;
}