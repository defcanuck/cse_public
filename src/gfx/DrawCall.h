#pragma once

#include "ClassDef.h"
#include "gfx/Types.h"
#include "gfx/TextureHandle.h"
#include "gfx/ShaderHandle.h"
#include "gfx/Color.h"

#include <unordered_map>

namespace cs
{

	CLASS_DEFINITION_REFLECT(DrawCallOverrides)
	public:
		DrawCallOverrides() { }

		std::map<std::string, ShaderHandlePtr> shaderOverrideHandles;
		TextureStages textureOverrideHandles;
	};

	CLASS_DEFINITION(DrawCall)

	public:
		
		typedef std::function<void()> PreDraw;
		typedef std::function<void()> PostDraw;
		typedef std::function<void()> UniformCallback;

		DrawCall();
		DrawCall(const DrawCall& rhs);

		DrawCall& operator=(const DrawCall& other)
		{
			this->tag = other.tag;
			this->type = other.type;
			this->offset = other.offset;
			this->count = other.count;
			this->shaderHandle = other.shaderHandle;
			this->textures = other.textures;
			this->indexType = other.indexType;
			this->indices = other.indices;
			this->blend = other.blend;
			this->srcBlend = other.srcBlend;
			this->dstBlend = other.dstBlend;
			this->depthTest = other.depthTest;
			this->depthFunc = other.depthFunc;
			this->color = other.color;
			this->scissor = other.scissor;
			this->scissorRect = other.scissorRect;
			this->cullFace = other.cullFace;
			this->frontFace = other.frontFace;
			this->depthWrite = other.depthWrite;
			this->layer = other.layer;
			this->postCallback = other.postCallback;
			this->preCallback = other.preCallback;
            this->instanceIndex = other.instanceIndex;
            
			return *this;
		}

		std::string tag;
		DrawType type;
		uint32 offset;
		uint32 count;
		ShaderHandlePtr shaderHandle;
		TextureStages textures;
		
		Type indexType;
		std::vector<uint16> indices;

		bool blend;
		BlendType srcBlend;
		BlendType dstBlend;

		bool depthTest;
		DepthType depthFunc;
		bool depthWrite;

		ColorB color;

		bool scissor;
		RectI scissorRect;

		CullFace cullFace;
		FrontFace frontFace;
		int32 layer;

		PostDraw postCallback;
		PreDraw preCallback;
		UniformCallback uniformCallback;
        int32 instanceIndex;

		void setTexture(TextureHandlePtr& texture, int stage)
		{
			textures[stage] = texture;
		}

		void bind(Geometry* geom, DrawCallOverrides* overrides);
		void setUniforms(ColorF tint = ColorF::White);

/*
		static void execute(std::vector<DrawCallPtr>& drawCalls, DrawCallOverrides* overrides, ColorF tint = ColorF::White);
*/
	};
}
