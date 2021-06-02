#pragma once

#include "ClassDef.h"

namespace cs
{
	CLASS_DEFINITION(VertexArrayObject)
	public:
		VertexArrayObject() { }
		virtual ~VertexArrayObject() { }

		virtual void bind() = 0;

	private:

	END_CLASS()
}