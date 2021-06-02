#pragma once

#include "os/InputField.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(InputField_iOS, InputField)
	public:
		InputField_iOS(const RectF& rect, InputFilterPtr& inputFilter) :
			InputField(rect, inputFilter) { }


	protected:

		virtual void beginInputImpl();
		virtual void endInputImpl();

	};
}
