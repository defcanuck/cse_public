#include "PCH.h"

#include "gfx/VertexDeclaration.h"
#include "gfx/RenderInterface.h"

namespace cs
{
	VertexDeclaration::VertexDeclaration()
		: stride(0)
	{

	}

	VertexDeclaration::VertexDeclaration(AttributeMap& attribs)
		: attributes(attribs)
		, stride(0)
	{
		this->calcStride();
	}
}
