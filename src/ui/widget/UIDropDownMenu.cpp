#include "PCH.h"

#include "ui/widget/UIDropDownMenu.h"
#include "ui/behavior/UIClickBehavior.h"
#include "ui/UITextElement.h"

#include "os/FileManager.h"

#include <sstream>

namespace cs
{

	const std::string UIDropDownMenu::kDefaultChildTexture = "arrow.png";

	void UIDropDownMenu::build()
	{
		if (initialized)
			return;

		float32 yp = 0;
		uint32 ctr = 0;

		
		for (auto it : this->options)
		{
			UIDropDownMenuOption& option = it;
		
			std::stringstream ss;
			ss << this->name << "_" << ctr++;
			UITextElementPtr element = CREATE_CLASS(UITextElement, ss.str(), it.text);
			this->invokeInitCallback(ElementTypeItem, element.get());

			if (option.child)
			{
				CallbackArg0<void>::CallbackFunc triggerFunc = std::bind(&UIDropDownMenu::toggle, option.child.get());
				cs::UIClickBehaviorPtr click = CREATE_CLASS(UIClickBehavior, triggerFunc);
				element->addBehavior(click);
				
				this->addChild(option.child);
				const RectF& cur_rect = this->getScreenRect();
				
				option.child->setXPosition(cur_rect.size.w, HAlignLeft);
				option.child->hide();

				std::stringstream str;
				str << "arrow_" << ctr;
				UIElementPtr texElem = CREATE_CLASS(UIElement, str.str());
				texElem->setWidth(20.0f, cs::SpanPixels);
				texElem->setHeight(20.0f, cs::SpanPixels);
				texElem->setPosition(vec2(0.0f, 0.0f), cs::HAlignRight, cs::VAlignCenter);
				texElem->setTexture("arrow.png");
				element->addChild(texElem);


			} else {

				cs::UIClickBehaviorPtr click = CREATE_CLASS(UIClickBehavior, option.callbacks);
				click->addCallback(this->closeParent);
				element->addBehavior(click);
			}
			
			RectF rect = element->getScreenRect(this->screen_rect);
			yp -= rect.size.h + 1.0f;
			
			element->setYPosition(yp);

			this->addChild(element);
			optionElements.push_back(std::static_pointer_cast<UIElement>(element));
		}

		initialized = true;
	}

	void UIDropDownMenu::reset()
	{
		log::info("reset");
	}
}