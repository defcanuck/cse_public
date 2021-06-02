#pragma once

#include "ClassDef.h"
#include "gfx/Attribute.h"
#include "gfx/DrawCall.h"
#include "gfx/VertexDeclaration.h"
#include "gfx/Types.h"

#include "global/Values.h"

#include <vector>

namespace cs
{

	CLASS_DEFINITION(GeometryData)
	
	public:
		GeometryData() :
			storage(BufferStorageStatic),
			vertexSize(0),
			indexSize(0) { }

		VertexDeclaration decl;
		BufferStorage storage;

		inline void pushVertexData(const vec2& vec)
		{
			this->vertexData.push_back(vec.x);
			this->vertexData.push_back(vec.y);
		}

		inline void pushVertexData(const vec3& vec)
		{
			this->vertexData.push_back(vec.x);
			this->vertexData.push_back(vec.y);
			this->vertexData.push_back(vec.z);
		}
    
        inline void pushVertexData(const vec4& vec)
        {
            this->vertexData.push_back(vec.x);
            this->vertexData.push_back(vec.y);
            this->vertexData.push_back(vec.z);
            this->vertexData.push_back(vec.w);
        }

		inline void pushVertexData(const ColorF& color)
		{
			this->vertexData.push_back(color.r);
			this->vertexData.push_back(color.g);
			this->vertexData.push_back(color.b);
			this->vertexData.push_back(color.a);
		}

		std::vector<float32> vertexData;
		size_t vertexSize;

		std::vector<uint16> indexData;
		size_t indexSize;

		std::vector<DrawCallPtr> drawCalls;
	
	};
}

