#pragma once

#include "ClassDef.h"

#include "ui/UIGlobal.h"
#include "ui/UIBehavior.h"
#include "global/Event.h"
#include "audio/SoundManager.h"

#include <functional>

namespace cs
{

	CLASS_DEFINITION_DERIVED(UIClickBehavior, UIBehavior)
	public:
		
		UIClickBehavior() { }

		UIClickBehavior(Event& e)
		{
			this->onClick = e;
		}

		UIClickBehavior(std::vector<CallbackPtr>& fcs)
		{
			this->onClick = fcs;
		}

		UIClickBehavior(CallbackArg0<void>::CallbackFunc& fn)
		{ 
			this->addCallback(fn);
		}

		UIClickBehavior(CallbackArg0<void>::callbackFunc fn)
		{ 
			CallbackArg0<void>::CallbackFunc func = std::bind(fn);
			this->addCallback(func);
		}

		UIClickBehavior(CallbackPtr callback)
		{
			this->onClick.addCallback(callback);
		}

		virtual bool onEnter(UIElement* element, UIElementClick& click);
		virtual bool onUpdate(UIElement* element, vec2& screen_pos);

		void setClickSound(const std::string& name);

		void addCallback(CallbackArg0<void>::CallbackFunc& fn)
		{ 
			CallbackPtr arg0 = std::make_shared<CallbackArg0<void>>(fn);
			this->onClick.addCallback(arg0);
		}
		
		virtual bool canProcessDisabled() const { return false; }

		void invoke();

		Event onClick;
		SoundEffectPtr effect;

	};
}