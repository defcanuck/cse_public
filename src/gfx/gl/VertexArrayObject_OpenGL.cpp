#include "PCH.h"

#include "gfx/gl/VertexArrayObject_OpenGL.h"

namespace cs
{
	VertexArrayObject_OpenGL::VertexArrayObject_OpenGL()
	{
		GL_CHECK(glGenVertexArrays(1, &this->vao));
		GL_CHECK(glBindVertexArray(this->vao));
	}

	VertexArrayObject_OpenGL::~VertexArrayObject_OpenGL()
	{
		glDeleteVertexArrays(1, &this->vao);
	}

	void VertexArrayObject_OpenGL::bind()
	{
		GL_CHECK(glBindVertexArray(this->vao));
	}
}