#include "PCH.h"

#include "gfx/Uniform.h"
#include "gfx/RenderInterface.h"
#include "gfx/RenderTarget.h"

#include "os/LogManager.h"
#include "os/Allocator.h"

namespace cs
{

	const size_t kUniformPrimitiveSize[] =
	{
		sizeof(float32),
		sizeof(int32),
		sizeof(vec2),
		sizeof(vec3), 
		sizeof(vec4),
		sizeof(mat4)
	};

	Uniform::Uniform(const std::string& nname, UniformType ttype, int ccount, ShaderType st, bool allocate)
		: name(nname)
		, type(ttype)
		, count(ccount)
		, userData(nullptr)
        , shaderType(st)
		, allocated(allocate)
	{
		const size_t kAlignment = 16;
		this->size = kUniformPrimitiveSize[this->type] * this->count;

		if (this->allocated)
		{
			value.value_raw = alloc_aligned(this->size, kAlignment);
			memset(value.value_raw, 0, this->size);
		}
	}

	Uniform::~Uniform()
	{
		this->dealloc();
		
		if (this->userData)
		{
			delete[] this->userData;
		}
	}

	void Uniform::dealloc()
	{
		if (this->allocated && this->value.value_raw)
		{
			switch (type)
			{
				case UniformFloat: alloc_free(value.value_float); break;
				case UniformInt: alloc_free(value.value_int); break;
				case UniformVec2: alloc_free(value.value_vec2); break;
				case UniformVec3: alloc_free(value.value_vec3); break;
				case UniformVec4: alloc_free(value.value_vec4); break;
				case UniformMat4: alloc_free(value.value_mat4); break;
				default: assert(false); break;
			}
		}
		this->value.value_raw = nullptr;
	}

	void SharedUniform::addUniform(UniformPtr uniform)
	{
		if (!uniform) 
		{
			log::print(LogError, "Invalid uniform being added");
			return;
		}

		if (this->getUniform(uniform->getName())) 
		{
			log::print(LogError, "Duplicate uniform found in SharedUniforms");
			return;
		}

		this->uniforms[uniform->getName()] = uniform;
	}

	UniformPtr SharedUniform::getUniform(const std::string& name)
	{
		auto it = uniforms.find(name);
		if (it != uniforms.end())
			return it->second;

		return UniformPtr();
	}

	UniformDataTexture::UniformDataTexture(const std::string& nname, TextureStage st)
		: UniformDataInt(nname, getPhysicalStage(st), false, ShaderFragment)
		, defaultTexture(CREATE_CLASS(TextureHandle, RenderInterface::getDefaultTextureForStage(st)))
		, stage(st)
	{ 
	
	}

	UniformDataTexture::UniformDataTexture(const std::string& nname, TextureStage st, const std::string& defaultTexName)
		: UniformDataInt(nname, getPhysicalStage(st), false, ShaderFragment)
		, defaultTexture(CREATE_CLASS(TextureHandle, defaultTexName))
		, stage(st)
	{ 
	
	}

	UniformDataTexture::UniformDataTexture(const std::string& nname, TextureStage st, TexturePtr& texture)
		: UniformDataInt(nname, getPhysicalStage(st), false, ShaderFragment)
		, defaultTexture(CREATE_CLASS(TextureHandle, texture))
		, stage(st)
	{ 
	
	}

	UniformDataTexture::UniformDataTexture(const std::string& nname, TextureStage st, TextureHandlePtr& texture)
		: UniformDataInt(nname, getPhysicalStage(st), false, ShaderFragment)
		, defaultTexture(texture)
		, stage(st)
	{

	}

	UniformDataRenderTexture::UniformDataRenderTexture(const std::string& n, TextureStage st, RenderTargetType type, bool useDepth)
		: UniformDataTexture(n, st)
	{
		this->defaultTexture = CREATE_CLASS(TextureHandle, type, useDepth);
	}
}
