#include "PCH.h"

#include "gfx/ShaderHandle.h"
#include "global/ResourceFactory.h"
#include "gfx/RenderInterface.h"

#if defined(CS_METAL)
    #include "gfx/metal/BufferObject_Metal.h"
    #include "MTLRenderInterface.h"
#endif

namespace cs
{

	BEGIN_META_CLASS(ShaderHandle)
		ADD_MEMBER_RESOURCE(shader);
	SET_MEMBER_CALLBACK_POST(&ShaderHandle::onShaderChanged);
		ADD_MEMBER(textures);
		ADD_MEMBER(uniforms);
	END_META()


	ShaderHandle::ShaderHandle(const std::string& str)
	{
		ShaderResourcePtr shaderResource = std::static_pointer_cast<ShaderResource>(
			ResourceFactory::getInstance()->loadResource<ShaderResource>(str));

		if (shaderResource.get())
		{
			this->shader = shaderResource;
			this->onShaderChanged();
		}
	}

	void ShaderHandle::bind(ShaderBindParams& params)
	{
		if (!this->shader.get())
			return;

		RenderInterface::getInstance()->setCurrentShader(this->shader);
		for (auto it : this->uniforms)
		{
			// if (dirty) it->clear();
			it->bind();
		}

		ShaderUniformBindParams uniformParams;

#if defined(CS_METAL)

		UniformBufferMapping* mapping = nullptr;
		if (params.index >= uniformBuffer.size())
		{
			uniformBuffer.push_back(UniformBufferMapping());
			mapping = &uniformBuffer.back();
			for (int32 i = 0; i < ShaderMAX; ++i)
			{
				ShaderPtr shader_it = this->shader->getShader((ShaderType) i);
				if (shader_it->getUniformSize() > 0)
				{
					mapping->buffer[i].setSize(shader_it->getUniformSize());
					mapping->buffer[i].buffer = RenderInterface::getInstance()->createBufferObject(BufferTypeUniform);
					mapping->buffer[i].buffer->alloc(mapping->buffer[i].paddedSize, nullptr, BufferStorageDynamic);

					uniformParams.uniformBufferArray[i] = mapping->lock(ShaderType(i));
				}
			}
		}
		else
		{
			mapping = &uniformBuffer[params.index];
			for (uint32 i = 0; i < ShaderMAX; ++i)
			{
				if (mapping->buffer[i].buffer.get())
				{
					uniformParams.uniformBufferArray[i] = mapping->lock(ShaderType(i));
				}
			}
		}
#endif

		this->shader->bind(params, uniformParams);

#if defined(CS_METAL)
		for (uint32 i = 0; i < ShaderMAX; ++i)
		{
			if (mapping->buffer[i].buffer.get())
			{
				mapping->unlock(ShaderType(i));
			}
		}
#endif

		for (auto it : this->textures)
		{
			uint32 stage = getPhysicalStage((TextureStage)it.first);
			it.second->bind(stage);
		}
	}

	void ShaderHandle::populateTextureStages(TextureStages& stages)
	{
		for (auto it : this->textures)
		{
			if (!this->ignoredTextures.test((TextureStage)it.first))
			{
				stages[it.first] = it.second;
			}
		}
	}

	std::string ShaderHandle::getName() const
	{
		if (this->shader.get())
		{
			return this->shader->getName();
		}
		return "unbound";
	}


	ShaderUniformPtr ShaderHandle::getUniform(const std::string& uniform_name)
	{
		ShaderUniformMap::iterator it = this->uniformLookup.find(uniform_name);
		if (it == this->uniformLookup.end())
			return ShaderUniformPtr();
		return it->second;
	}

	bool ShaderHandle::equals(const ShaderHandlePtr& rhs)
	{
		if (!this->shader)
			return false;

		if (!this->shader->equals(rhs->getShader()))
			return false;

		for (auto it : this->uniformLookup)
		{
			ShaderUniformPtr rhsUniform = rhs->getUniform(it.first);
			if (!rhsUniform.get())
				return false;

			if (!rhsUniform->equals(it.second))
				return false;
		}

		return true;
	}

	void ShaderHandle::onPostLoad(const LoadFlagMask& flags)
	{
		this->remapUniforms();
		this->refreshTextures();

		for (auto it : this->textures)
		{
			it.second->onPostLoad(flags);
		}
	}

	void ShaderHandle::onNew()
	{
		this->refreshTextures();
	}

	void ShaderHandle::onShaderChanged()
	{
		this->onChanged.invoke();
		this->refreshTextures();
		this->refreshUniforms();
#if defined(CS_METAL)
		this->refreshUniformBuffer();
#endif

	}

	void ShaderHandle::ignoreTextureStage(TextureStage stage)
	{
		this->ignoredTextures.set(stage);
	}

	void ShaderHandle::refreshTextures()
	{
		this->textures.clear();
		if (this->shader)
		{
			this->shader->populateTextureStages(this->textures, this->ignoredTextures);
		}
	}

	void ShaderHandle::refreshUniforms()
	{
		this->uniforms.clear();
		if (this->shader)
		{
			this->shader->populateMutableUniforms(this->uniforms);
		}

		this->uniformLookup.clear();
		for (auto it : this->uniforms)
			this->uniformLookup[it->getName()] = it;
	}

	void ShaderHandle::remapUniforms()
	{
		if (this->shader)
		{
			this->shader->mapMutableUniforms(this->uniforms);
		}

		this->uniformLookup.clear();
		for (auto it : this->uniforms)
			this->uniformLookup[it->getName()] = it;
	}


	template <>
	bool ShaderHandle::setUniformInternal<float32>(ShaderUniformPtr& ptr, const float32& value)
	{
		assert(ptr->getType() == UniformFloat);
		ShaderUniformFloatPtr float_ptr = std::dynamic_pointer_cast<ShaderUniformFloat>(ptr);
		if (!float_ptr.get())
			return false;
		float_ptr->setValue(static_cast<float32>(value));
		return true;
	}

	template <>
	bool ShaderHandle::setUniformInternal<double>(ShaderUniformPtr& ptr, const double& value)
	{
		switch (ptr->getType())
		{
		case UniformFloat:
		{
			ShaderUniformFloatPtr float_ptr = std::dynamic_pointer_cast<ShaderUniformFloat>(ptr);
			if (!float_ptr.get())
				return false;
			float_ptr->setValue(static_cast<float32>(value));
			return true;
		}
		case UniformInt:
		{
			ShaderUniformIntPtr int_ptr = std::dynamic_pointer_cast<ShaderUniformInt>(ptr);
			if (!int_ptr.get())
				return false;
			int_ptr->setValue(static_cast<int32>(value));
			return true;
		}
		default:
			assert(false);
		}
		return false;
	}

	template <>
	bool ShaderHandle::setUniformInternal<vec2>(ShaderUniformPtr& ptr, const vec2& value)
	{
		ShaderUniformVec2Ptr vec2_ptr = std::dynamic_pointer_cast<ShaderUniformVec2>(ptr);
		if (!vec2_ptr.get())
		{
			log::error("Invalid cast on ", ptr->getName());
			return false;
		}
		vec2_ptr->setValue(value);
		return true;
	}

	template <>
	bool ShaderHandle::setUniformInternal<vec3>(ShaderUniformPtr& ptr, const vec3& value)
	{
		ShaderUniformVec3Ptr vec3_ptr = std::dynamic_pointer_cast<ShaderUniformVec3>(ptr);
		if (!vec3_ptr.get())
		{
			log::error("Invalid cast on ", ptr->getName());
			return false;
		}
		vec3_ptr->setValue(value);
		return true;
	}

	template <>
	bool ShaderHandle::setUniformInternal<vec4>(ShaderUniformPtr& ptr, const vec4& value)
	{
		ShaderUniformVec4Ptr vec4_ptr = std::dynamic_pointer_cast<ShaderUniformVec4>(ptr);
		if (!vec4_ptr.get())
		{
			log::error("Invalid cast on ", ptr->getName());
			return false;
		}
		vec4_ptr->setValue(value);
		return true;
	}

	template <>
	bool ShaderHandle::setUniformInternal<mat4>(ShaderUniformPtr& ptr, const mat4& value)
	{
		ShaderUniformMat4Ptr mat4_ptr = std::dynamic_pointer_cast<ShaderUniformMat4>(ptr);
		if (!mat4_ptr.get())
		{
			log::error("Invalid cast on ", ptr->getName());
			return false;
		}
		mat4_ptr->setValue(value);
		return true;
	}

#if defined(CS_METAL)

	void* ShaderHandle::getUniformBuffer(ShaderType type, uint32 index)
	{
		assert(index < this->uniformBuffer.size());
		UniformBufferMapping& mapping = this->uniformBuffer[index];
		if (mapping.exists(type))
		{
			BufferObject_MetalPtr mtlBuffer = std::static_pointer_cast<BufferObject_Metal>(mapping.buffer[type].buffer);
			if (mtlBuffer.get())
			{
				return mtlBuffer->getBufferObject();
			}
		}
		return nullptr;
	}

	void* ShaderHandle::getPipelineDescriptor(const ShaderBindParams& bindParams)
	{
		if (!shader.get())
		{
			return nullptr;
		}
		return this->shader->getPipelineDescriptor(bindParams);
	}

	void ShaderHandle::refreshUniformBuffer()
	{
		if (this->shader.get())
		{
			for (auto& mapping : this->uniformBuffer)
			{
				for (int32 i = 0; i < ShaderMAX; ++i)
				{
					ShaderPtr shader_it = this->shader->getShader((ShaderType) i);
					if (shader_it->getUniformSize() > 0)
					{
						size_t bufSize = (mapping.exists(ShaderType(i))) ? mapping.buffer[i].realSize : 0;
						bool sizeMismatch =  bufSize != shader_it->getUniformSize();

						if (sizeMismatch || !mapping.exists(ShaderType(i)))
						{
							if (mapping.buffer[i].buffer.get())
							{
								mapping.buffer[i].buffer->dealloc();
								mapping.buffer[i].buffer = nullptr;
								mapping.buffer[i].realSize = 0;
								mapping.buffer[i].paddedSize = 0;
							}

							mapping.buffer[i].buffer = RenderInterface::getInstance()->createBufferObject(BufferTypeUniform);
							mapping.buffer[i].buffer->alloc(shader_it->getUniformSize(), nullptr, BufferStorageDynamic);
                            MTLIncrementDataScope(MTLDebugCounter_UniformBuffer);
						}
					}
				}
			}
		}
	}

	void* ShaderHandle::UniformBufferMapping::lock(ShaderType type)
	{
		assert(this->buffer[type].buffer.get());
		return this->buffer[type].buffer->lock(BufferAccessWrite);
	}

	void ShaderHandle::UniformBufferMapping::unlock(ShaderType type)
	{
		assert(this->buffer[type].buffer.get());
		this->buffer[type].buffer->unlock();
	}

#endif

}
