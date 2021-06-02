#pragma once

#include "ClassDef.h"
#include "gfx/ShaderResource.h"
#include "gfx/Uniform.h"
#include "gfx/BufferObject.h"
#include "gfx/ShaderBinding.h"

namespace cs
{

    class Geometry;
    
    

	CLASS_DEFINITION_REFLECT(ShaderHandle)
	public:
		ShaderHandle()
			: shader(nullptr)
		{ }

		ShaderHandle(const ShaderResourcePtr sh)
			: shader(sh)
		{ 
			this->onShaderChanged();
		}

		ShaderHandle(const ShaderHandlePtr rhs)
			: shader(rhs->shader)
			, ignoredTextures(rhs->ignoredTextures)
			, textures(rhs->textures)
			, uniforms(rhs->uniforms)
		{ 
			this->onShaderChanged();
		}

		ShaderHandle(const std::string& str);

		void bind(ShaderBindParams& params);

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags);

		void ignoreTextureStage(TextureStage stage);

		ShaderResourcePtr& getShader() { return this->shader; }
		bool hasShader() const { return this->shader.get() != nullptr; }
		std::string getName() const;

		bool equals(const ShaderHandlePtr& rhs);

		void setTextureIgnored(TextureStage stage) { this->ignoredTextures.set(stage); }
		void clearTextureIgnored(TextureStage stage) { this->ignoredTextures.unset(stage); }
        
        void populateTextureStages(TextureStages& stages);

		void onShaderChanged();
		Event onChanged;

		ShaderUniformPtr getUniform(const std::string& uniform_name);

		template <class T>
		bool setUniformValue(const std::string& name, const T& value)
		{
			ShaderUniformPtr ptr = this->getUniform(name);
			if (!ptr.get())
			{
				log::error("Cannot find uniform of name:", name);
				return false;
			}

			return setUniformInternal<T>(ptr, value);
		}
    
#if defined(CS_METAL)
    
        void* getPipelineDescriptor(const ShaderBindParams& bindParams);
        void* getUniformBuffer(ShaderType type, uint32 index = 0);

#endif

	protected:

		void refreshTextures();
		void refreshUniforms();
		void remapUniforms();

		ShaderResourcePtr shader;
		TextureStageMask ignoredTextures;

		TextureStages textures;
		ShaderUniformList uniforms;
		ShaderUniformMap uniformLookup;

#if defined(CS_METAL)
    
        struct UniformBufferMapping
        {
            struct BufferWrapper
            {
                BufferWrapper()
                    : buffer(nullptr)
                    , realSize(0)
                    , paddedSize(0)
                { }
                
                ~BufferWrapper()
                {
                    
                }
                
                BufferObjectPtr buffer;
                size_t realSize;
                size_t paddedSize;
                
                inline void setSize(size_t rawSize)
                {
                    this->realSize = rawSize;
                    this->paddedSize = realSize + 16 - (realSize % 16);
                }
            };
            
            UniformBufferMapping()
            {
                for (int32 i = 0; i < ShaderMAX; ++i) buffer[i].buffer = nullptr;
            }
            
            void* lock(ShaderType type);
            void unlock(ShaderType type);
            
            inline bool exists(ShaderType type) const { return this->buffer[type].buffer.get() != nullptr; }
            
            BufferWrapper buffer[ShaderMAX];
        };
        //BufferObjectPtr uniformBuffer[ShaderMAX];
        std::vector<UniformBufferMapping> uniformBuffer;
    
        void refreshUniformBuffer();
    
#endif
    
		template <class T>
		bool setUniformInternal(ShaderUniformPtr& ptr, const T& value)
		{
			assert(false);
			return false;
		}
	};

	template <>
	bool ShaderHandle::setUniformInternal<float32>(ShaderUniformPtr& ptr, const float32& value);

	template <>
	bool ShaderHandle::setUniformInternal<double>(ShaderUniformPtr& ptr, const double& value);

	template <>
	bool ShaderHandle::setUniformInternal<vec2>(ShaderUniformPtr& ptr, const vec2& value);

	template <>
	bool ShaderHandle::setUniformInternal<vec3>(ShaderUniformPtr& ptr, const vec3& value);

	template <>
	bool ShaderHandle::setUniformInternal<vec4>(ShaderUniformPtr& ptr, const vec4& value);

	template <>
	bool ShaderHandle::setUniformInternal<mat4>(ShaderUniformPtr& ptr, const mat4& value);



}
