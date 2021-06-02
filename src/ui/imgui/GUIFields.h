#pragma once

#include "global/Values.h"
#include "math/Rect.h"

#include <vector>

namespace cs
{

	struct GUIValue;

	struct GUIFieldEntry
	{
		GUIFieldEntry();
		GUIFieldEntry(const GUIFieldEntry& rhs);
		void operator=(const GUIFieldEntry& rhs);
		~GUIFieldEntry();

		GUIValue* field;
		float32 indent;
	};

	struct GUIFields
	{
		~GUIFields() 
		{
			this->clear(); 
		}

		GUIFields() { }
		GUIFields(GUIFields& rhs) : list(rhs.list) { }

		bool batch(const RectF& rect);
		void add(GUIValue* value, float32 indent = 0.0f);
		void clear();

		typedef std::vector<GUIFieldEntry> FieldsList;
		FieldsList list;
	};
}