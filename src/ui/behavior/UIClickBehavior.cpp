#include "PCH.h"

#include "ui/behavior/UIClickBehavior.h"

namespace cs
{
	bool UIClickBehavior::onEnter(UIElement* element, UIElementClick& click) 
	{ 
		if (click.state == TouchPressed)
		{
			this->invoke();
			if (this->effect.get())
			{
				this->effect->play();
			}
			return true;
		}
		return false;
	}

	bool UIClickBehavior::onUpdate(UIElement* element, vec2& screen_pos) 
	{ 
		return true;
	}

	void UIClickBehavior::invoke()
	{
		this->onClick.invoke();
		if (this->effect.get())
		{
			this->effect->play();
		}
	}

	void UIClickBehavior::setClickSound(const std::string& name)
	{
		this->effect = SoundManager::getInstance()->getSound(name);
	}
}