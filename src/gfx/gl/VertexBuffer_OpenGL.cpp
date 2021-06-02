#include "PCH.h"

#include "gfx/gl/VertexBuffer_OpenGL.h"
#include "gfx/gl/OpenGL.h"
#include "gfx/Types.h"

namespace cs
{
	bool isNormalized(AttributeType type)
	{
		return (type == AttributeType::AttribNormal || type == AttributeType::AttribTangent || type == AttributeType::AttribColor);
	}

	VertexBuffer_OpenGL::~VertexBuffer_OpenGL()
	{
			
	}

	void VertexBuffer_OpenGL::enableAttribute(AttributeType type)
	{
		GL_CHECK(glEnableVertexAttribArray((GLuint) type));
	}

	void VertexBuffer_OpenGL::setAttributeBuffer(AttributeType type, Type dataType, size_t offset, uint32 count, size_t stride)
	{
		GL_CHECK(glVertexAttribPointer((GLuint) type, count, kTypeConvert[dataType], isNormalized(type), stride, (GLvoid*)(((char*) nullptr) + offset)));
	}

	void VertexBuffer_OpenGL::bindAttributes(VertexDeclaration& decl)
	{

		size_t stride = decl.getStride();
		for (size_t i = 0; i < AttribMAX; i++)
		{
			AttributeType type = AttributeType(i);
			const Attribute* attribute = decl.getAttrib(type);
			if (attribute)
			{
				this->enableAttribute(type);
				this->setAttributeBuffer(type, (Type) attribute->dataType, attribute->offset, attribute->count, stride);
			}
		}
	}
}
