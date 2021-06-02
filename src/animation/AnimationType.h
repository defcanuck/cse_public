#pragma once

#include "ClassDef.h"
#include "animation/Animator.h"
#include "geom/Volume.h"
#include "gfx/Color.h"
#include "math/GLM.h"
#include "math/Transform.h"

namespace cs
{
	class SceneNode;

	CLASS_DEFINITION_REFLECT(AnimationPlayBase)
	public:
		virtual AnimationType getType() { return AnimationTypeNone; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationPlayLoop, AnimationPlayBase)
	public:
		virtual AnimationType getType() { return AnimationTypeLoop; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationPlayBounce, AnimationPlayBase)
	public:
		virtual AnimationType getType() { return AnimationTypeBounce; }
	};

	CLASS_DEFINITION_REFLECT(AnimationScaleBase)
	public:
		virtual AnimationScaleType getType() { return AnimationScaleTypeNone; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationScaleCenter, AnimationScaleBase)
	public:
		virtual AnimationScaleType getType() { return AnimationScaleTypeCenter; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationScaleLeft, AnimationScaleBase)
	public:
		virtual AnimationScaleType getType() { return AnimationScaleTypeLeft; }
	};


	template <class T>
	class AnimatorLerpTyped : public LerpAnimator<T>
	{
	public:

		AnimatorLerpTyped()
			: LerpAnimator<T>(AnimatorLerpTyped<T>::getDefaultMin(), AnimatorLerpTyped<T>::getDefaultMax(), 1.0f)
		{ }

		AnimatorLerpTyped(const T& mmin, const T& mmax, float32 max_time, float32 smooth)
			: LerpAnimator<T>(mmin, mmax, max_time, smooth)
		{ }

		static T getDefaultMin() { return T(); }
		static T getDefaultMax() { return T(); }
	};

#if defined(CS_IOS)

	template <>
    vec3 AnimatorLerpTyped<vec3>::getDefaultMin();
	template <>
    vec3 AnimatorLerpTyped<vec3>::getDefaultMax();

	template <>
	vec2 AnimatorLerpTyped<vec2>::getDefaultMin();
	template <>
	vec2 AnimatorLerpTyped<vec2>::getDefaultMax();

	template <>
    float32 AnimatorLerpTyped<float32>::getDefaultMin();
	template <>
    float32 AnimatorLerpTyped<float32>::getDefaultMax();
	
	template <>
    ColorB AnimatorLerpTyped<ColorB>::getDefaultMin();
	template <>
    ColorB AnimatorLerpTyped<ColorB>::getDefaultMax();

#else

	template <>
	vec3 AnimatorLerpTyped<vec3>::getDefaultMin() { return kZero3; }
	template <>
	vec3 AnimatorLerpTyped<vec3>::getDefaultMax() { return kOne3; }

	template <>
	vec2 AnimatorLerpTyped<vec2>::getDefaultMin() { return kZero2; }
	template <>
	vec2 AnimatorLerpTyped<vec2>::getDefaultMax() { return kOne2; }

	template <>
	float32 AnimatorLerpTyped<float32>::getDefaultMin() { return 0.0f; }
	template <>
	float32 AnimatorLerpTyped<float32>::getDefaultMax() { return 1.0f; }

	template <>
	ColorB AnimatorLerpTyped<ColorB>::getDefaultMin() { return ColorB::Black; }
	template <>
	ColorB AnimatorLerpTyped<ColorB>::getDefaultMax() { return ColorB::White; }

#endif
	
	typedef AnimatorLerpTyped<vec3> AnimatorLerpVec3Impl;
	typedef AnimatorLerpTyped<vec2> AnimatorLerpVec2Impl;
	typedef AnimatorLerpTyped<float32> AnimatorLerpFloatImpl;
	typedef AnimatorLerpTyped<ColorB> AnimatorLerpColorImpl;
	
	CLASS_DEFINITION_DERIVED_REFLECT(AnimatorLerpVec3, AnimatorLerpVec3Impl)
	public:
		AnimatorLerpVec3() 
			: AnimatorLerpVec3Impl()
		{ }

		virtual void getVolumes(VolumeList& volumes);

	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimatorLerpVec2, AnimatorLerpVec2Impl)
	public:
		AnimatorLerpVec2()
			: AnimatorLerpVec2Impl()
		{ }

		AnimatorLerpVec2(const vec2& mmin, const vec2& mmax, float32 max_time, float32 smooth)
			: AnimatorLerpVec2Impl(mmin, mmax, max_time, smooth)
		{ }

		virtual void getVolumes(VolumeList& volumes);

	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimatorLerpFloat, AnimatorLerpFloatImpl)
	public:
		AnimatorLerpFloat()
			: AnimatorLerpFloatImpl()
		{ }

		virtual void getVolumes(VolumeList& volumes) { }

	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimatorLerpColor, AnimatorLerpColorImpl)
	public:
		AnimatorLerpColor()
			: AnimatorLerpColorImpl()
		{ }

		AnimatorLerpColor(const ColorB& start_color, const ColorB& end_color, float32 max_time, float32 smooth)
			: AnimatorLerpColorImpl(start_color, end_color, max_time, smooth)
		{ }

		virtual void getVolumes(VolumeList& volumes) { }
	};

	CLASS_DEFINITION_REFLECT(AnimatedValue)
	public:
		AnimatedValue() 
			: animPlay(CREATE_CLASS(AnimationPlayLoop))
			, baseInstance(nullptr)
			, speed(1.0f)
		{ }

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual void reset(SceneNode* node, bool active = false);

		virtual void onResetAnimation() { }
		virtual bool process(float32 dt);
		virtual void apply(SceneNode* node) { assert(false); }

		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes) { }

		void onSpeedChanged();

		virtual void onAnimInit() { }

		virtual void setAnimationType(AnimationType type);

	protected:

		AnimationPlayBasePtr animPlay;
		AnimationBase* baseInstance;
		float32 speed;
	};

	typedef std::vector<AnimatedValuePtr> AnimatedValueList;

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationPosition, AnimatedValue)
	public:

		AnimationPosition()
			: AnimatedValue()
			, posAnimation(CREATE_CLASS(AnimatorLerpVec3))
		{ }

		virtual void onResetAnimation();
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);
		virtual void apply(SceneNode* node);

		virtual void onAnimInit();

		virtual void setAnimationType(AnimationType type) 
		{
			this->instance.setType(type);
			BASECLASS::setAnimationType(type);
		}

		AnimatorPtr posAnimation;
		Animation<vec3> instance;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationRotation, AnimatedValue)
	public:

		AnimationRotation()
			: AnimatedValue()
			, rotAnimation(CREATE_CLASS(AnimatorLerpFloat))
			, axis(kDefalutZAxis)
		{ }

		virtual void onResetAnimation();
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);
		virtual void apply(SceneNode* node);

		virtual void onAnimInit();

		virtual void setAnimationType(AnimationType type) 
		{ 
			this->instance.setType(type); 
			BASECLASS::setAnimationType(type);
		}

		AnimatorPtr rotAnimation;
		Animation<float32> instance;
		vec3 axis;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationColor, AnimatedValue)
	public:

		AnimationColor()
			: AnimatedValue()
			, colorAnimation(CREATE_CLASS(AnimatorLerpColor))
		{ }

		AnimationColor(const ColorB& start_color, const ColorB& end_color, float32 max_time, float32 smooth)
			: AnimatedValue()
			, colorAnimation(CREATE_CLASS(AnimatorLerpColor, start_color, end_color, max_time, smooth))
		{ }

		virtual void onResetAnimation();
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);
		virtual void apply(SceneNode* node);

		virtual void onAnimInit();

		virtual void setAnimationType(AnimationType type) 
		{ 
			this->instance.setType(type);
			BASECLASS::setAnimationType(type);
		}

		AnimatorPtr colorAnimation;
		Animation<ColorB> instance;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationSize, AnimatedValue)
		public:

			AnimationSize()
				: AnimatedValue()
				, sizeAnimation(CREATE_CLASS(AnimatorLerpVec2))
				, scaleType(CREATE_CLASS(AnimationScaleLeft))
			{ }

			AnimationSize(const vec2& sz_min, const vec2& sz_max, float32 max_time, float32 smooth)
				: AnimatedValue()
				, sizeAnimation(CREATE_CLASS(AnimatorLerpVec2, sz_min, sz_max, max_time, smooth))
				, scaleType(CREATE_CLASS(AnimationScaleLeft))
			{ }

			virtual void onResetAnimation();
			virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);
			virtual void apply(SceneNode* node);

			virtual void onAnimInit();

			virtual void setAnimationType(AnimationType type) 
			{ 
				this->instance.setType(type); 
				BASECLASS::setAnimationType(type);
			}

			void setAnimationScaleType(AnimationScaleType type);

			AnimatorPtr sizeAnimation;
			Animation<vec2> instance;
			AnimationScaleBasePtr scaleType;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AnimationTextureUV, AnimatedValue)
	public:

		AnimationTextureUV()
			: AnimatedValue()
			, posAnimation(CREATE_CLASS(AnimatorLerpVec2))
			, szAnimation(CREATE_CLASS(AnimatorLerpVec2))
		{ }

		AnimationTextureUV(const vec2& pos_min, const vec2& pos_max, const vec2& sz_min, const vec2& sz_max, float32 max_time, float32 smooth)
			: AnimatedValue()
			, posAnimation(CREATE_CLASS(AnimatorLerpVec2, pos_min, pos_max, max_time, smooth))
			, szAnimation(CREATE_CLASS(AnimatorLerpVec2, sz_min, sz_max, max_time, smooth))
		{ }

		virtual void onResetAnimation();
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);
		virtual void apply(SceneNode* node);

		virtual void onAnimInit();

		virtual void setAnimationType(AnimationType type) 
		{ 
			this->instance.setType(type); 
			BASECLASS::setAnimationType(type);
		}

		AnimatorPtr szAnimation;
		AnimatorPtr posAnimation;
		Animation<vec2> instance;
	};

}
