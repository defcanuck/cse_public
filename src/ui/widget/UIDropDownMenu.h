#pragma once

#include "ClassDef.h"
#include "ui/widget/UIWidget.h"
#include "global/Event.h"

namespace cs
{
	

	CLASS_DEFINITION_DERIVED(UIDropDownMenu, UIWidget)
	public:

		static const std::string kDefaultChildTexture;

		struct UIDropDownMenuOption
		{
			std::string text;
			CallbackList callbacks;
			UIDropDownMenuPtr child;
		};

		typedef std::vector<UIDropDownMenuOption> UIDropDownMenuOptions;

		enum ElementType
		{
			ElementTypeItem
		};

		UIDropDownMenu(const std::string name, UIDropDownMenuOptions& op) :
			UIWidget(name),
			options(op),
			initialized(false),
			childTexture(kDefaultChildTexture)
		{
			this->setVertexColor(ColorB::Clear);
		}

		virtual ColorB getCurrentTint(BatchDrawData* parent = nullptr) const { return ColorB::White; }
		
		virtual void trigger()
		{
			this->enabled = true;
			this->visible = true;
		}

		virtual void hide()
		{
			this->enabled = false;
			this->visible = false;
		}

		virtual void toggle()
		{
			this->enabled = !this->enabled;
			this->visible = !this->visible;
		}

		void setCloseParent(CallbackArg0<void>::CallbackFunc& cp) { this->closeParent = cp; }
		void setChildTexture(const std::string& fileName) { this->childTexture = fileName; }

		void setOptions(UIDropDownMenuOptions& opt)
		{
			this->options = opt;
			this->build();
		}

	protected:
		
		virtual void build();
		virtual void reset();

		bool initialized;
		RectF offsetRect;
		UIDropDownMenuOptions options;
		std::vector<UIElementPtr> optionElements;

		CallbackArg0<void>::CallbackFunc closeParent;
		std::string childTexture;

	};	
}