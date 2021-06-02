#include "PCH.h"
#include "ui/UISlider.h"
#include "global/Utils.h"

namespace cs
{
	UISlider::UISlider(const std::string& n)
		: UIElement(n)
		, tickElement(nullptr)
		, currTickPercent(0.0f)
		, lastTickPercent(0.0f)
		, lastTickDrag(0.0f)
		, onChanged(nullptr)
		, tickDrag(0.0f)
		, fullscreenTouch(false)
	{
		initializeTick();
	}

	void UISlider::batch(
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		const UIBatchProcessData& data,
		UIBatchProcessInfo& info)
	{
		this->lastTickDrag = lerp<float32>(this->currTickPercent, this->lastTickDrag, this->tickDrag);
		this->tickElement->setPosition(this->lastTickDrag, SpanPercent);

		BASECLASS::batch(display_list, numVertices, numIndices, data, info);
	}

	void UISlider::setTickPercent(float32 pct)
	{
		this->currTickPercent = clamp(0.0f, 100.0f - this->tickWidth, (100.0f * pct) - (this->tickWidth * 0.5f));;
		this->lastTickDrag = this->currTickPercent;
	}

	bool UISlider::onCursor(ClickInput input, ClickParams& params, UIClickResults& results, uint32 depth)
	{

		if (!this->enabled)
		{
			return false;
		}
	
		if (input == ClickPrimary)
		{
			if (this->fullscreenTouch || rectContains<float32, vec2>(this->screen_rect, params.position))
			{

				vec2 percent = rectPercent<float32, vec2>(this->screen_rect, params.position);
				
				UIElementClick result;
				result.state = params.state;
				result.inputType = input;
				result.absolutePos = params.position;
				result.relativePos = params.position;
				result.relativePct = percent;
				result.depth = depth;
				results[this] = result;

				this->currTickPercent = clamp(0.0f, 100.0f - this->tickWidth, (100.0f * percent.x) - (this->tickWidth * 0.5f));
				
				if (this->currTickPercent != this->lastTickPercent)
				{
					if (this->onChanged.get())
					{
						float curTickPct = this->currTickPercent / (100.0f - this->tickWidth);
						(*this->onChanged.get())(curTickPct);
					}
					this->lastTickPercent = this->currTickPercent;
				}
				
				// if fullscreen touch is enabled, we want to cascade taps
				if (this->fullscreenTouch)
					return false;
				else
					return true;
			}
			else
			{

				this->onExit(input);
				for (auto it : this->children)
					(*it).onExit(input);
			}
		}
	

		return false;
	}

	void UISlider::initializeTick()
	{
		const float32 kDefaultTickWidth = 5.0f;
		const float32 kDefaultTickHeight = 95.0f;

		if (this->tickElement.get())
		{
			this->removeChild(this->tickElement);
			this->tickElement = nullptr;
		}

		this->tickElement = CREATE_CLASS(UIElement, "tick");

		// Default to center
		this->tickElement->setXPosition(50.0f, HAlignLeft, SpanPercent);
		this->tickElement->setYPosition(0.0f, VAlignCenter, SpanPercent);

		this->tickWidth = kDefaultTickWidth;
		this->tickHeight = kDefaultTickHeight;

		// use default dimensions
		this->tickElement->setWidth(kDefaultTickWidth, SpanPercent);
		this->tickElement->setHeight(kDefaultTickHeight, SpanPercent);

		this->addChild(this->tickElement);
	}

	void UISlider::setTickWidth(float32 tw)
	{
		this->tickElement->setWidth(tw, SpanPercent);
		this->tickWidth = tw;
	}
}