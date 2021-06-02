#include "PCH.h"

#include "gfx/ShaderResource.h"
#include "gfx/Shader.h"
#include "gfx/RenderInterface.h"

#if defined(CS_METAL)
    #include "gfx/metal/Shader_Metal.h"
    #include "gfx/metal/ShaderProgram_Metal.h"
#endif

#include "global/ResourceFactory.h"
#include "os/FileManager.h"

#include <algorithm>

namespace cs
{
	BEGIN_META_RESOURCE(ShaderResource)

	END_META()
    
    ShaderResource::ShaderResource(const std::string& n, ShaderBucket b, const ShaderParams& shader_params)
        : Resource(n)
        , bucket(b)
        , program(nullptr)
        , params(shader_params)
    {
        this->init();
    }
    
	ShaderResource::~ShaderResource()
	{

	}
    
    ShaderPtr ShaderResource::getShader(ShaderType type)
    {
        if (this->program.get())
        {
            return this->program->getShader(type);
        }
        return ShaderPtr();
    }

	void ShaderResource::refresh()
	{

		ShaderPtr vshader = RenderInterface::getInstance()->loadShader(ShaderVertex, "", this->params.printSource);
        if (vshader.get())
            vshader->compile(this->getName(), this->params.vertexSource->getSource(), this->params.printSource);

		ShaderPtr fshader = RenderInterface::getInstance()->loadShader(ShaderFragment, "", this->params.printSource);
        if (fshader.get())
            fshader->compile(this->getName(), this->params.fragmentSource->getSource(), this->params.printSource);

        if (vshader.get() && fshader.get())
        {
            this->program = RenderInterface::getInstance()->createShaderProgram();
            if (this->program.get())
            {
                this->program->addShader(vshader);
                this->program->addShader(fshader);

                for (const auto& it : this->params.attributes)
                {
                    const AttributeType type = (AttributeType)it.first;
                    const std::string& attrib_name = it.second;
                    if (!this->program->bindAttributeLocation(attrib_name.c_str(), type))
                    {
                        log::error("bindAttributeLocation error for shader ", this->getName());
                    }
                    else
                    {
                        attributes.push_back(type);
                    }
                }
                this->program->link(&params);

				ShaderBindParams bindParams;
                this->program->bind(bindParams);
            }
            else
            {
                log::error("Error creating program!");
            }
        }
    }

	void ShaderResource::init()
	{
		this->refresh();
	}

	void ShaderResource::bind(const ShaderBindParams& bindParams, ShaderUniformBindParams& uniformParams)
	{
		// Bind the shader
		this->program->bind(bindParams, this->name.c_str());

		// Set uniforms
		for (auto it : this->params.uniforms)
		{
			UniformPtr& uniform = it;
			if (!uniform)
				continue;
            
            ShaderType shaderType = uniform->getShaderType();
            void* bufferDst = nullptr;
            
#if defined(CS_METAL)
            // Textures aren't updated as uniforms in Metal
            if (uniform->isTexture() || !uniformParams.uniformBufferArray[shaderType])
                continue;
            
            ShaderProgram_MetalPtr mtlProgram = std::static_pointer_cast<ShaderProgram_Metal>(this->program);
            bufferDst = mtlProgram->getBufferOffset(uniform->getName(), shaderType, uniformParams.uniformBufferArray[shaderType]);
            if (!bufferDst)
            {
                continue;
            }
#endif
            
			switch (uniform->getType()) 
			{
				case UniformFloat:
					this->program->setUniformValueArray(
						uniform->getName().c_str(),
						static_cast<float32*>(uniform->getData()),
						uniform->getCount(),
                        1, bufferDst);
					break;
				case UniformInt:
					this->program->setUniformValueArray(
						uniform->getName().c_str(),
						static_cast<int32*>(uniform->getData()),
						uniform->getCount(), bufferDst);
					break;
				case UniformVec2:
					this->program->setUniformValueArray(
						uniform->getName().c_str(),
						static_cast<vec2*>(uniform->getData()),
						uniform->getCount(), bufferDst);
					break;
				case UniformVec3:
					this->program->setUniformValueArray(
						uniform->getName().c_str(),
						static_cast<vec3*>(uniform->getData()),
						uniform->getCount(), bufferDst);
					break;
				case UniformVec4:
					this->program->setUniformValueArray(
						uniform->getName().c_str(),
						static_cast<vec4*>(uniform->getData()),
						uniform->getCount(), bufferDst);
					break;
				case UniformMat4:
					this->program->setUniformValueArray(
						uniform->getName().c_str(),
						static_cast<mat4*>(uniform->getData()),
						uniform->getCount(), bufferDst);
					break;
                default:
                    assert(false);
			}
		}
	}

	void ShaderResource::refreshShaders()
	{
		struct local
		{
			static void refreshShader(ShaderResourcePtr& shader_resource, uintptr_t* data)
			{
				shader_resource->refresh();
			}
		};
		ResourceFactory::getInstance()->performOperation<ShaderResource>(&local::refreshShader, nullptr);
	}

	bool ShaderResource::equals(const ShaderResourcePtr& rhs) const
	{
		return this->program.get() == rhs->getProgram().get();
	}

	void ShaderResource::update(const Uniform::UpdateParams& updateParams)
	{
		for (UniformList::iterator it = this->params.uniforms.begin(); it != this->params.uniforms.end(); ++it)
		{
			UniformPtr& uniformPtr = *it;

			if (!uniformPtr->canOverload())
				uniformPtr->update(updateParams);
		}
	}

	void ShaderResource::getTextureStages(std::vector<TextureStage>& stages, TextureStageMask& ignoredMask)
	{
		for (auto it : this->params.uniforms)
		{
			if (it->isTexture())
			{
				UniformDataTexturePtr tex_uniform = std::static_pointer_cast<UniformDataTexture>(it);
				if (!ignoredMask.test(tex_uniform->getStage()))
					stages.push_back(tex_uniform->getStage());
			}
		}
	}

	void ShaderResource::populateTextureStages(TextureStages& textures, TextureStageMask& ignoredMask)
	{
		for (auto it : this->params.uniforms)
		{
			if (!it.get())
			{
				log::error("Empty shader uniform - invalid!");
				continue;
			}

			if (it->isTexture())
			{
				UniformDataTexturePtr tex_uniform = std::static_pointer_cast<UniformDataTexture>(it);
				if (!ignoredMask.test(tex_uniform->getStage()))
					textures[tex_uniform->getStage()] = tex_uniform->getDefaultTexture();
			}
		}
	}

	void ShaderResource::populateMutableUniforms(ShaderUniformList& uniforms)
	{
		for (auto it : this->params.uniforms)
		{
			if (!it->canOverload())
				continue;

			switch (it->getType())
			{
				case UniformFloat:
				{
					UniformDataFloatPtr float_uniform = std::static_pointer_cast<UniformDataFloat>(it);
					uniforms.push_back(CREATE_CLASS(ShaderUniformFloat, float_uniform, float_uniform->getDataValue()));
				}
				break;
				case UniformInt:	
				{
					UniformDataIntPtr int_uniform = std::static_pointer_cast<UniformDataInt>(it);
					uniforms.push_back(CREATE_CLASS(ShaderUniformInt, int_uniform, int_uniform->getDataValue()));
				}
				break;
				case UniformVec2:
				{
					UniformDataVec2Ptr vec2_uniform = std::static_pointer_cast<UniformDataVec2>(it);
					uniforms.push_back(CREATE_CLASS(ShaderUniformVec2, vec2_uniform, vec2_uniform->getDataValue()));
				}
				break;
				case UniformVec3:
				{
					UniformDataVec3Ptr vec3_uniform = std::static_pointer_cast<UniformDataVec3>(it);
					uniforms.push_back(CREATE_CLASS(ShaderUniformVec3, vec3_uniform, vec3_uniform->getDataValue()));
				}
				break;
				case UniformVec4:
				{
					UniformDataVec4Ptr vec4_uniform = std::static_pointer_cast<UniformDataVec4>(it);
					uniforms.push_back(CREATE_CLASS(ShaderUniformVec4, vec4_uniform, vec4_uniform->getDataValue()));
				}
				break;
				case UniformMat4:
				{
					UniformDataMat4Ptr float_uniform = std::static_pointer_cast<UniformDataMat4>(it);
					uniforms.push_back(CREATE_CLASS(ShaderUniformMat4, float_uniform, float_uniform->getDataValue()));
				}
				break;
                default:
                    break;
			}
		}
	}

	void ShaderResource::mapMutableUniforms(ShaderUniformList& uniforms)
	{
		for (auto it : uniforms)
		{
			UniformPtr foundUniform;
			for (auto uniform : this->params.uniforms)
			{
				if (uniform->canOverload() && uniform->getName() == it->getName())
				{
					foundUniform = uniform;
					break;
				}
			}

			if (!foundUniform.get())
			{
				log::error("No uniform to map ", it->getName());
				continue;
			}
				
			switch (foundUniform->getType())
			{
				case UniformFloat:
				{
					ShaderUniformFloatPtr float_dst = std::static_pointer_cast<ShaderUniformFloat>(it);
					float_dst->setUniform(std::static_pointer_cast<UniformDataFloat>(foundUniform));
					break;
				}
				break;
				case UniformInt:
				{
					ShaderUniformIntPtr int_dst = std::static_pointer_cast<ShaderUniformInt>(it);
					int_dst->setUniform(std::static_pointer_cast<UniformDataInt>(foundUniform));
					break;
				}
				break;
				case UniformVec2:
				{
					ShaderUniformVec2Ptr vec2_dst = std::static_pointer_cast<ShaderUniformVec2>(it);
					vec2_dst->setUniform(std::static_pointer_cast<UniformDataVec2>(foundUniform));
					break;
				}
				break;
				case UniformVec3:
				{
					ShaderUniformVec3Ptr vec3_dst = std::static_pointer_cast<ShaderUniformVec3>(it);
					vec3_dst->setUniform(std::static_pointer_cast<UniformDataVec3>(foundUniform));
					break;
				}
				break;
				case UniformVec4:
				{
					ShaderUniformVec4Ptr vec4_dst = std::static_pointer_cast<ShaderUniformVec4>(it);
					vec4_dst->setUniform(std::static_pointer_cast<UniformDataVec4>(foundUniform));
					break;
				}
				break;
				case UniformMat4:
				{
					ShaderUniformMat4Ptr mat4_dst = std::static_pointer_cast<ShaderUniformMat4>(it);
					mat4_dst->setUniform(std::static_pointer_cast<UniformDataMat4>(foundUniform));
					break;
				}
                default:
                    assert(false);
			}
		}
	}

#if defined(CS_METAL)
    void* ShaderResource::getPipelineDescriptor(const ShaderBindParams& bindParams)
    {
        if (!this->program.get())
            return nullptr;
        
        return this->program->getPipelineDescriptor(bindParams);
    }
    
    const std::map<uint32, uint32>* ShaderResource::getTextureRemap()
    {
        if (!this->program.get())
            return nullptr;
        
        ShaderProgram_MetalPtr mtlProgram = std::static_pointer_cast<ShaderProgram_Metal>(this->program);
        if (mtlProgram.get())
        {
            return mtlProgram->getTextureStageMap();
        }
        return nullptr;
    }
#endif
}
