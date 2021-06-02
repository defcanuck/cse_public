#pragma once

#include "gfx/gl/BufferObject_OpenGL.h"
#include "gfx/VertexDeclaration.h"
#include "gfx/gl/OpenGL.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(VertexBuffer_OpenGL, BufferObject_OpenGL)
	public:
		VertexBuffer_OpenGL()
			: BufferObject_OpenGL(BufferTypeVertex)
		{ }

		virtual ~VertexBuffer_OpenGL();

		virtual void bindAttributes(VertexDeclaration& decl);

	private:

		void enableAttribute(AttributeType type);
		void setAttributeBuffer(AttributeType type, Type dataType, uint32 offset, uint32 count, uint32 stride);

	END_CLASS()
}