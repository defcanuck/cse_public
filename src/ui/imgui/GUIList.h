#pragma once

#include "ui/imgui/GUIElement.h"

namespace cs
{
	class GUIList;
	
	CLASS_DEFINITION(GUIListPopulator)
	public:

		virtual void populate(GUIList* list) = 0;
	};

	CLASS_DEFINITION_DERIVED(GUIList, GUIElement)
	public:
		GUIList(const std::string& name, GUIListPopulatorPtr p)
			: GUIElement(name)
			, shouldRefresh(false)
			, populator(p) 
			, numColumns(1)
			, initialized(false)
		{ 
			
		}

		bool prepareGUI(bool inWindow, size_t depth);

		template <class T>
		void setPopulator(T& populator)
		{
			this->clearFields();
			GUIListPopulatorPtr pop = std::static_pointer_cast<GUIListPopulator>(populator);
			this->populator = pop;
			this->setPopulatorImpl();
		}


		void refresh()
		{
			shouldRefresh = true;
		}

		void setColumns(int32 c) { this->numColumns = c; }

	protected:

		void setPopulatorImpl();

	private:

		bool shouldRefresh;

		friend class GUIListPopulator;
		GUIListPopulatorPtr populator;
		int32 numColumns;
		bool initialized;
	};

}