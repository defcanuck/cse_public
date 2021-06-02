#pragma once

#include "ClassDef.h"

#include "os/Input.h"
#include "gfx/Color.h"

namespace cs
{
	class UIElement;

	struct UIElementClick
	{
		TouchState state;
		ClickInput inputType;
		vec2 absolutePos;
		vec2 relativePct;
		vec2 relativePos;
		uint32 depth;
	};

	typedef std::map<UIElement*, UIElementClick> UIClickResults;

	CLASS_DEFINITION(UIBehavior)	
	public:
		UIBehavior() : enabled(true) { }
		virtual bool onEnter(UIElement* element, UIElementClick& click) { return false; }
		virtual bool onUpdate(UIElement* element, vec2& screen_pos) { return false; }
		virtual void onExit(UIElement* element, ClickInput input) { }
		virtual void process(UIElement* element, float32 dt) { }

		void setEnabled(bool e) { this->enabled = e; }
		bool getEnabled() const { return this->enabled; }

		virtual bool canProcessDisabled() const { return false; }
		virtual void onFirstFrame() { }

	protected:

		bool enabled;
	};

	CLASS_DEFINITION_DERIVED(UIDragBehavior, UIBehavior)
	public:

		UIDragBehavior() :
			UIBehavior(),
			dragging(false),
			offset(0.0f, 0.0f) { }

		virtual bool onEnter(UIElement* element, UIElementClick& click);
		virtual bool onUpdate(UIElement* element, vec2& screen_pos);
		virtual void onExit(UIElement* element, ClickInput input);

	private:

		bool dragging;
		vec2 offset;

	};
}