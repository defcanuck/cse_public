#pragma once

#pragma once

#include "ClassDef.h"

#include "ecs/comp/Component.h"
#include "animation/AnimationType.h"
#include "geom/Volume.h"
#include "scene/SceneNode.h"

namespace cs
{
	struct SystemUpdateParams;

	struct AnimationUpdateParams
	{
		float32 dt;
		CameraPtr camera;
		SceneNode* node;
	};

	CLASS_DEFINITION_REFLECT(AnimationMethod)
	public:
		AnimationMethod()
		{ }

		virtual void reset(SceneNode* node, bool active = false) { }
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll) { }
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes) { }
		virtual bool process(AnimationUpdateParams* params) { return false; }
		virtual void resetAnimations() { }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationMethodValue, AnimationMethod)
	public:
		AnimationMethodValue()
			: AnimationMethod()
		{ }

		virtual void reset(SceneNode* node, bool active = false);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);
		virtual bool process(AnimationUpdateParams* params);

		virtual void resetAnimations();

		template <class T>
		void addAnimationValue(std::shared_ptr<T>& anim)
		{
			this->valueList.push_back(std::static_pointer_cast<AnimatedValue>(anim));
		}

	protected:

		AnimatedValueList valueList;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationMethodPath, AnimationMethod)

	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationComponent, Component)
	public:
		AnimationComponent();
		virtual ~AnimationComponent() { }

		virtual void reset(bool active = false);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		void process(SystemUpdateParams* params);

		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);

		void resetAnimations();

		template <class T>
		void setAnimationMethod(std::shared_ptr<T>& m)
		{
			this->method = std::static_pointer_cast<AnimationMethod>(m);
		}

		void clearAnimationMethod()
		{
			this->method = nullptr;
		}

		LuaCallbackPtr onAnimationComplete;

	protected:

		bool active;
		AnimationMethodPtr method;
	
	};
}