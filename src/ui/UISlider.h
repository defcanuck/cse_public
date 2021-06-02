#pragma once

#include "ui/UIElement.h"
#include "scripting/LuaCallback.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(UISlider, UIElement)
	public:
		UISlider(const std::string& n);

		virtual void batch(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const UIBatchProcessData& data,
			UIBatchProcessInfo& info);

		virtual bool onCursor(ClickInput input, ClickParams& params, UIClickResults& results, uint32 depth);

		UIElementPtr getTickElement() { return this->tickElement; }

		LuaCallbackPtr onChanged;

		void setTickWidth(float32 tw);
		void setTickDrag(float32 td) { this->tickDrag = td; }
		void setTickPercent(float32 pct);

		float32 getTickPercent() const { return this->currTickPercent; }
		float32 getRawPercent() const { return this->lastTickDrag / (100.0f - this->tickWidth); }
		void setFullscreenTouch(bool enabled) { this->fullscreenTouch = enabled; }

	protected:

		float32 tickWidth;
		float32 tickHeight;
		float32 currTickPercent;
		float32 lastTickPercent;
		float32 tickDrag;
		float32 lastTickDrag;

		UIElementPtr tickElement;
		bool fullscreenTouch;

		void initializeTick();

	};
}