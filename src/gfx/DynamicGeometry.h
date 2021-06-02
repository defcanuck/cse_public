#pragma once

#include "math/GLM.h"
#include "gfx/Geometry.h"
#include "gfx/ShaderResource.h"
#include "gfx/TextureHandle.h"

#include <functional>

namespace cs
{


	typedef size_t(*vertexUpdateFunc)(uchar*, size_t, VertexDeclaration&);
	typedef size_t(*indexUpdateFunc)(uchar*, size_t);
	typedef void(*adjustDrawCallFunc)(int32, std::vector<DrawCallPtr>&);
	typedef size_t(*getVertexSizeFunc)();
	typedef size_t(*getIndexSizeFunc)();

	CLASS_DEFINITION_DERIVED(DynamicGeometry, Geometry)
	public:

		typedef std::function<size_t(uchar*, size_t, VertexDeclaration&)> VertexUpdateFunc;
		typedef std::function<size_t(uchar*, size_t)> IndexUpdateFunc;
		typedef std::function<void(int32, std::vector<DrawCallPtr>&)> AdjustDrawCallFunc;
		typedef std::function<size_t()> GetVertexSizeFunc;
		typedef std::function<size_t()> GetIndexSizeFunc;

		DynamicGeometry(const GeometryDataPtr& init_data);
		
		virtual void update();
		virtual void draw(DrawCallOverrides* overrides, int32 index = -1, ColorF tint = ColorF::White);

		void setVertexUpdateFunc(VertexUpdateFunc& func);
		void setVertexUpdateFunc(vertexUpdateFunc func = nullptr);

		void setIndexUpdateFunc(IndexUpdateFunc& func);
		void setIndexUpdateFunc(indexUpdateFunc func = nullptr);

		void setDrawCallAdjustFunc(AdjustDrawCallFunc& func);
		void setDrawCallAdjustFunc(adjustDrawCallFunc func = nullptr);

		void setVertexBufferSizeFunc(GetVertexSizeFunc& func);
		void setVertexBufferSizeFunc(getVertexSizeFunc func = nullptr);

		void setIndexBufferSizeFunc(GetIndexSizeFunc& func);
		void setIndexBufferSizeFunc(getIndexSizeFunc func = nullptr);

	

	private:

		VertexUpdateFunc vertexUpdate;
		IndexUpdateFunc indexUpdate;
		AdjustDrawCallFunc adjustDraw;
		GetVertexSizeFunc vertexBufSize;
		GetIndexSizeFunc indexBufSize;

	};
}