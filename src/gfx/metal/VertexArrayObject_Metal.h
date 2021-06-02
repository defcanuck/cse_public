#pragma once

#include "gfx/VertexArrayObject.h"
#include "gfx/gl/OpenGL.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(VertexArrayObject_OpenGL, VertexArrayObject)
	public:
		VertexArrayObject_OpenGL();
		virtual ~VertexArrayObject_OpenGL();

		virtual void bind();

	private:

		GLuint vao;

	END_CLASS()
}