#pragma once

#include "ClassDef.h"

#include "global/Utils.h"
#include "global/Event.h"
#include "gfx/Color.h"
#include "geom/Volume.h"

namespace cs
{
	enum AnimationType
	{
		AnimationTypeNone = -1,
		AnimationTypeLoop,
		AnimationTypeBounce,
		//...
		AnimationTypeMAX
	};

	enum AnimationState
	{
		AnimationStateNone = -1,
		AnimationStatePlaying,
		AnimationStatePaused,
		AnimationStateFinished,
		//...
		AnimationStateNAX
	};

	class AnimationBase
	{
	public:
		
		AnimationBase(AnimationType t = AnimationTypeNone, float32 s = 1.0f)
			: type(t)
			, currentTime(0.0f)
			, speed(s)
		{ }

		AnimationBase(const AnimationBase& rhs)
			: type(rhs.type)
			, currentTime(rhs.currentTime)
			, speed(rhs.speed)
		{ }

		AnimationType getType() const { return this->type; }
		void setType(AnimationType t) 
		{
			this->type = t;
			this->reset();
		}

		virtual bool process(const float32& dt) 
		{ 
			this->currentTime += dt * this->speed;
			return this->evaluateStep(dt);
		}

		virtual bool evaluateStep(const float32& dt)
		{
			return false;
		}

		virtual bool isAnimDone() const
		{
			// no real animation, so true
			return true; 
		}

		virtual void reset()
		{
			this->currentTime = 0.0f;
		}

		void setSpeed(float32 s) { this->speed = s; }

	protected:

		AnimationType type;
		float32 currentTime;
		float32 speed;

	};

	CLASS_DEFINITION_REFLECT(Animator)
	public:
		Animator() { }

		virtual void getVolumes(VolumeList& selectable_volumes) { }
		virtual void onAnimationChanged() { this->onChanged.invoke(); }

		Event onChanged;
	};

	template <class T>
	class AnimatorTyped : public Animator
	{
	public:
		AnimatorTyped(const float32& max_time)
			: maxTime(max_time)
		{

		}

		virtual T getStartValue() { return T(); }

		virtual void evaluate(T& value, float32& cur_time) { }
		float32 getMaxTime() const { return this->maxTime; }
		bool isAnimDone(const float32& cur_time) const
		{ 
			return this->maxTime <= cur_time; 
		}

	protected:

		float32 maxTime;
	};

	template <class T>
	class Animation : public AnimationBase
	{
	public:

		Animation()
			: AnimationBase(AnimationTypeNone)
			, value(T())
			, timeWarp(1.0f)
		{
			// log::error("Uninitialized Animation!");
		}

		Animation(std::shared_ptr<AnimatorTyped<T>>& anim, AnimationType t, float32 sp = 1.0f)
			: AnimationBase(t, sp)
			, animator(anim)
			, value(T())
			, timeWarp(1.0f)
		{
			this->value = anim->getStartValue();
		}

		Animation(std::shared_ptr<AnimatorTyped<T>>& anim, const Animation<T>& rhs)
			: AnimationBase(rhs)
			, animator(anim)
		{ }

		void setInitialValue(const T& val)
		{
			this->value = val;
		}

		Animation(const Animation<T>& rhs)
			: AnimationBase(rhs.type)
		{
			this->animator = rhs.animator;
			this->value = rhs.value;
			this->timeWarp = rhs.timeWarp;
		}

		void operator=(const Animation<T>& rhs)
		{
			AnimationBase::operator=(rhs);
			this->animator = rhs.animator;
			this->value = rhs.value;
			this->timeWarp = rhs.timeWarp;
		}

		T getValue() 
		{ 
			return this->value; 
		}

		virtual bool hasAnim() const
		{
			return this->animator.get() != nullptr;
		}

		virtual bool isAnimDone() const
		{
			if (!this->animator.get())
			{
				// no animator
				return true;
			}

			switch (this->type)
			{
				case AnimationTypeNone:
				{
					float max_time = this->animator->getMaxTime();
					return this->currentTime >= max_time;
				}
				case AnimationTypeLoop:
				case AnimationTypeBounce:
					return false;
                default:
                    log::warning("Unknown type!");
                    break;
			}
			return false;
		}

		virtual bool process(const float32& dt)
		{
			float32 dt_adjusted = dt * this->timeWarp;
			bool ret = AnimationBase::process(dt_adjusted);
			assert(this->animator);
			this->animator->evaluate(this->value, this->currentTime);
			return ret;
		}

		T evaluate(std::shared_ptr<AnimatorTyped<T>>& injectAnimator)
		{
			assert(injectAnimator.get());
			T injectVal;
			injectAnimator->evaluate(injectVal, this->currentTime);
			return injectVal;
		}

		virtual bool evaluateStep(const float32& dt)
		{
			assert(this->animator);
			switch (this->type)
			{
				case AnimationTypeNone:
					this->currentTime = clamp<float32>(0.0f, this->animator->getMaxTime(), this->currentTime);
					return this->currentTime >= this->animator->getMaxTime();
				case AnimationTypeLoop:
					if (this->animator->isAnimDone(this->currentTime))
					{
						this->currentTime = this->currentTime - this->animator->getMaxTime();
					}
					return false;
				case AnimationTypeBounce:
					if (this->animator->isAnimDone(this->currentTime))
					{
						float32 offset = this->currentTime - this->animator->getMaxTime();
						this->currentTime = this->animator->getMaxTime() - offset;
						this->timeWarp = -1.0f;
					}
					else if (this->currentTime < 0.0f)
					{
						this->currentTime = -this->currentTime;
						this->timeWarp = 1.0f;
					}
					return false;
                default:
                    log::warning("Unknown animator");
                    break;
			}
			return false;
		}

		std::shared_ptr<AnimatorTyped<T>> animator;
		T value;
		float32 timeWarp;
	};

	template <class T>
	class DummyAnimator : public AnimatorTyped<T>
	{
	public:

		DummyAnimator(const float32 max_time, const float32 sm = 1.0f)
			: AnimatorTyped<T>(max_time)
			, percent(0.0f)
			, smooth(sm)
		{ }

		virtual void evaluate(T& value, float32& cur_time)
		{
			value = clamp(0.0f, 1.0f, pow(cur_time * (1.0f / this->maxTime), smooth));
		}

		static Animation<T> createAnimation(
			const float32 max_time,
			const float32 smooth,
			AnimationType type = AnimationTypeNone)
		{
			std::shared_ptr<AnimatorTyped<T>> animator = std::static_pointer_cast<AnimatorTyped<T>>(std::make_shared<DummyAnimator<T>>(max_time, smooth));
			return Animation<T>(animator, type);
		}

		float32 percent;
		float32 smooth;
	};

	template <class T>
	class LerpAnimator : public AnimatorTyped<T>
	{
	public:

		LerpAnimator(const T& minv, const T& maxv, const float32 max_time, const float32 sm = 1.0f)
			: AnimatorTyped<T>(max_time)
			, minValue(minv)
			, maxValue(maxv)
			, smooth(sm)
		{ }

		virtual T getStartValue() { return this->minValue; }
		virtual void evaluate(T& value, float32& cur_time)
		{
			float32 pct = clamp(0.0f, 1.0f, pow(cur_time * (1.0f / this->maxTime), smooth));
			value = lerp(this->minValue, this->maxValue, pct);
		}

		static Animation<T> createAnimation(
			const T& minv, 
			const T& maxv, 
			const float32 max_time,
			const float32 smooth,
			AnimationType type = AnimationTypeNone)
		{
			std::shared_ptr<AnimatorTyped<T>> animator = std::static_pointer_cast<AnimatorTyped<T>>(std::make_shared<LerpAnimator<T>>(minv, maxv, max_time, smooth));
			return Animation<T>(animator, type);
		}

		T minValue;
		T maxValue;
		float32 smooth;
	};

	class AnimationInstance
	{
	public:

		virtual void onBegin() { }
		virtual void onEnd() { }

		virtual void firstFrame() { }
		virtual void process(float32 dt) { }
		virtual bool isActive() const { return false; }
		virtual void reset() { }

	};

	template <class T>
	class AnimationInstanceTyped : public AnimationInstance
	{
	public:
		AnimationInstanceTyped(Animation<T> anim)
			: AnimationInstance()
			, animation(anim)
		{

		}

		virtual void process(float32 dt)
		{
			this->animation.process(dt);
		}

		virtual void reset()
		{
			this->animation.reset();
			this->animation.process(0.0f);
		}

		virtual bool isActive() const { return !this->animation.isAnimDone(); }

		Animation<T> animation;
	};

	typedef std::shared_ptr<AnimationInstance> AnimationInstancePtr;

	template <class T>
	class AnimationReference : public AnimationInstanceTyped<T>
	{
	public:

		AnimationReference(Animation<T>& anim, T* ptr)
			: AnimationInstanceTyped<T>(anim)
			, ref_ptr(ptr)
		{

		}

		virtual void process(float32 dt)
		{
			this->animation.process(dt);
			if (this->ref_ptr)
			{
				(*this->ref_ptr) = this->animation.getValue();
			}
		}

		T* ref_ptr;
	};

	template <class T>
	class AnimationCallback : public AnimationInstanceTyped<T>
	{
	public:
		AnimationCallback(Animation<T>& anim, CallbackArg1<void, const T&>& call)
			: AnimationInstanceTyped<T>(anim)
			, callback(call)
		{

		}

		virtual void process(float32 dt)
		{
			this->animation->process(dt);
			if (this->callback)
			{
				this->callback->invoke(this->animation->getValue());
			}
		}

		CallbackArg1<void, const T&> callback;
	};

	template <class T>
	inline AnimationInstancePtr createAnimationCallback(Animation<T>& anim, CallbackArg1<void, const T&>& call)
	{
		std::shared_ptr<AnimationCallback<T>> animation = std::make_shared<AnimationCallback<T>>(anim, call);
		return animation;
	}

	template <class T>
	inline AnimationInstancePtr createAnimationCallback(Animation<T>& anim, void(*func_ptr)(const T&))
	{
		typename CallbackArg1<void, T>::CallbackFunc func = std::bind(func_ptr, std::placeholders::_1);
		CallbackArg1<void, T> call(func);
		return createAnimationCallback<T>(anim, call);
	}

	template <class T, class Caller>
	inline AnimationInstancePtr createAnimationCallback(Animation<T>& anim, void(Caller::*func_ptr)(T), Caller* caller_ptr)
	{
		typename CallbackArg1<void, T>::CallbackFunc func = std::bind(func_ptr, caller_ptr, std::placeholders::_1);
		CallbackArg1<void, T> call(func);
		return createAnimationCallback<T>(anim, call);
	}

	template <class C>
	class AnimationSharedCallbackHandler
	{
	public:
		static void onBegin(std::shared_ptr<C>& object) { }
		static void onEnd(std::shared_ptr<C>& object) { }
	};

	template <class T, class C>
	class AnimationSharedCallback : public AnimationInstanceTyped<T>
	{
	public:
		AnimationSharedCallback(Animation<T>& anim, CallbackArg1<void, T>& call, std::shared_ptr<C>& ptr)
			: AnimationInstanceTyped<T>(anim)
			, callback(call)
			, object(ptr)
		{

		}

		virtual void firstFrame()
		{
			this->callback.invoke(this->animation.getValue());
		}

		virtual void onBegin()
		{
			AnimationSharedCallbackHandler<C>::onBegin(this->object);
		}

		virtual void onEnd()
		{
			AnimationSharedCallbackHandler<C>::onEnd(this->object);
		}

		virtual void process(float32 dt)
		{
			this->animation.process(dt);
			this->callback.invoke(this->animation.getValue());
		}

		CallbackArg1<void, T> callback;
		std::shared_ptr<C> object;
	};

	template <class T, class C>
	class AnimationNamedCallback : public AnimationInstanceTyped<T>
	{
	public:
		AnimationNamedCallback(Animation<T>& anim, const std::string& n, CallbackArg2<void, std::string, T>& call, std::shared_ptr<C>& ptr)
			: AnimationInstanceTyped<T>(anim)
			, name(n)
			, callback(call)
			, object(ptr)
		{

		}

		virtual void firstFrame()
		{
			// this->callback.invoke(this->name, this->animation.getValue());
		}

		virtual void onBegin()
		{
			AnimationSharedCallbackHandler<C>::onBegin(this->object);
		}

		virtual void onEnd()
		{
			AnimationSharedCallbackHandler<C>::onEnd(this->object);
		}

		virtual void process(float32 dt)
		{
			this->animation.process(dt);
			this->callback.invoke(this->name, this->animation.getValue());
		}

		std::string name;
		CallbackArg2<void, std::string, T> callback;
		std::shared_ptr<C> object;
	};

	template <class T, class C>
	inline AnimationInstancePtr createAnimationCallback(Animation<T>& anim, void(C::*func_ptr)(T), std::shared_ptr<C>& caller_ptr)
	{
		typename CallbackArg1<void, T>::CallbackFunc func = std::bind(func_ptr, caller_ptr.get(), std::placeholders::_1);
		CallbackArg1<void, T> call(func);
		std::shared_ptr<AnimationSharedCallback<T, C>> anim_instance = std::make_shared<AnimationSharedCallback<T, C>>(anim, call, caller_ptr);
		return anim_instance;
	}

	template <class T, class C>
	inline AnimationInstancePtr createNamedAnimationCallback(const std::string& name, Animation<T>& anim, void(C::*func_ptr)(std::string, T), std::shared_ptr<C>& caller_ptr)
	{
		typename CallbackArg2<void, std::string, T>::CallbackFunc func = std::bind(func_ptr, caller_ptr.get(), std::placeholders::_1, std::placeholders::_2);
		CallbackArg2<void, std::string, T> call(func);
		std::shared_ptr<AnimationNamedCallback<T, C>> anim_instance = std::make_shared<AnimationNamedCallback<T, C>>(anim, name, call, caller_ptr);
		return anim_instance;
	}

	class AnimationInstanceList
	{
	public:

		AnimationInstanceList() { }

		void onFirstFrame()
		{
			for (auto& inst : this->instances)
			{
				inst->reset();
				inst->firstFrame();
			}
		}

		bool process(float32 dt)
		{
			bool isActive = false;
			for (auto& it : this->instances)
			{
				it->process(dt);
				isActive = isActive || it->isActive();
			}
			return isActive;
		}

		void onBegin()
		{
			for (auto& it : this->instances)
				it->onBegin();
		}

		void onEnd(bool resetOnEnd = false)
		{
			std::vector<AnimationInstancePtr> copy_instances = this->instances;
			
			if (!resetOnEnd)
				this->instances.clear();
			
			for (auto& it : copy_instances)
				it->onEnd();

			if (resetOnEnd)
			{
				for (auto& inst : this->instances)
				{
					inst->reset();
				}
			}
		}

		std::vector<AnimationInstancePtr> instances;
	
	};

	typedef Animation<float32> FloatAnimation;
	typedef Animation<int32> IntAnimation;
	typedef Animation<vec2> Vec2Animation;
	typedef Animation<vec3> Vec3Animation;
	typedef Animation<ColorB> ColorBAnimation;
	typedef Animation<ColorF> ColorFAnimation;

	typedef LerpAnimator<float32> FloatLerpAnimator;
	typedef LerpAnimator<int32> IntLerpAnimator;
	typedef LerpAnimator<vec2> Vec2LerpAnimator;
	typedef LerpAnimator<vec3> Vec3LerpAnimator;
	typedef LerpAnimator<ColorB> ColorBLerpAnimator;
	typedef LerpAnimator<ColorF> ColorFLerpAnimator;

	typedef DummyAnimator<float32> PercentAnimator;

}
