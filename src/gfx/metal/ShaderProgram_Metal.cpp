#include "PCH.h"

#include "gfx/metal/ShaderProgram_Metal.h"
#include "gfx/metal/Shader_Metal.h"
#include "gfx/Geometry.h"

#include "MTLRenderInterface.h"

namespace cs
{

	void ShaderProgram_Metal::init()
	{
	
    }

	void ShaderProgram_Metal::free()
	{
        
	}

	bool ShaderProgram_Metal::valid() const
	{
        return false;
	}

	bool ShaderProgram_Metal::bindAttributeLocation(const std::string& name, AttributeType type)
	{
        ShaderPtr& vertexShader = shaders[ShaderVertex];
        if (vertexShader.get())
        {
            Shader_MetalPtr metalShader = std::static_pointer_cast<Shader_Metal>(vertexShader);
            void* ptr = metalShader->getShaderPtr();
            int idx = MTLGetVertexAttributeFromProgram(ptr, name.c_str());
            if (idx >= 0)
            {
                this->attributes[type] = idx;
                return true;
            }
            else
            {
                log::error("Cannot find name=>index mapping for attribute ", name);
                return false;
            }
        }
        return false;
    }

	void ShaderProgram_Metal::setUniformValueArray(const std::string& name, float32* value, uint32 count, uint32 precision, void* dst)
	{
        memcpy(dst, value, count * sizeof(float32));
	}

	void ShaderProgram_Metal::setUniformValueArray(const std::string& name, int32* value, uint32 count, void* dst)
	{
        memcpy(dst, value, count * sizeof(int32));
	}

	void ShaderProgram_Metal::setUniformValueArray(const std::string& name, vec2* value, uint32 count, void* dst)
	{
        memcpy(dst, value, count * sizeof(vec2));
	}

	void ShaderProgram_Metal::setUniformValueArray(const std::string& name, vec3* value, uint32 count, void* dst)
	{
        memcpy(dst, value, count * sizeof(vec3));
	}

	void ShaderProgram_Metal::setUniformValueArray(const std::string& name, vec4* value, uint32 count, void* dst)
	{
        memcpy(dst, value, count * sizeof(vec4));
	}

	void ShaderProgram_Metal::setUniformValueArray(const std::string& name, mat4* value, uint32 count, void* dst)
	{
        memcpy(dst, value, count * sizeof(mat4));
	}

	void ShaderProgram_Metal::link(const ShaderParams* params)
	{
        Shader_MetalPtr fragShader = std::static_pointer_cast<Shader_Metal>(this->getShader(ShaderFragment));
        if (fragShader.get())
        {
            for (const auto& it : params->uniforms)
            {
                if (it->isTexture())
                {
                    UniformDataTexturePtr tex_uniform = std::static_pointer_cast<UniformDataTexture>(it);
                    std::map<std::string, uint32>::iterator tex = fragShader->textureStageMap.find(tex_uniform->getName());
                    if (tex != fragShader->textureStageMap.end())
                    {
                        int logicalStage = tex_uniform->getStage();
                        int physicalStage = tex->second;
                        
                        this->textureStageMap[logicalStage] = physicalStage;
                    }
                }
            }
        }
	}
    
    ShaderProgram_Metal::ShaderBindParamMap* ShaderProgram_Metal::getParamMap(const VertexDeclaration& decl, VertexDeclaration& out_decl)
    {
        VertexDeclaration use_decl(decl);
        for (uint32 i = 0; i < use_decl.getNumAttributes(); ++i)
        {
            const Attribute* attrib = use_decl.getAttribAt(i);
            if (this->attributes.find(attrib->type) == this->attributes.end())
            {
                AttributeType removeType = attrib->type;
                use_decl.removeAttrib(removeType);
            }
        }
        ShaderVertexParamMap::iterator decl_iter = this->pipelineDescriptor.find(use_decl);
        
        ShaderBindParamMap* param_map = nullptr;
        if (decl_iter == this->pipelineDescriptor.end())
        {
            std::pair<VertexDeclaration, ShaderBindParamMap> toInsert(use_decl, ShaderBindParamMap());
            decl_iter = this->pipelineDescriptor.insert(toInsert).first;
            param_map = &decl_iter->second;
        }
        else
        {
            param_map = &decl_iter->second;
        }
        out_decl = use_decl;
        return param_map;
    }

    void* ShaderProgram_Metal::getPipelineDescriptor(const ShaderBindParams& bindParams)
    {
        VertexDeclaration use_decl;
        ShaderBindParamMap* param_map = this->getParamMap(bindParams.geom->getGeometryData()->decl, use_decl);
        assert(param_map);
        
        ShaderBindParamMap::iterator it = param_map->find(bindParams);
        if (it == (*param_map).end())
        {
            return createPipelineDescInternal(bindParams, param_map, use_decl);
        }
        return it->second;;
    }
    
    void* ShaderProgram_Metal::createPipelineDescInternal(const ShaderBindParams& bindParams, ShaderBindParamMap* param_map, const VertexDeclaration& decl, const char* tag)
    {
        ShaderPtr& vertexShader = this->shaders[ShaderVertex];
        ShaderPtr& fragmentShader = this->shaders[ShaderFragment];
        
        Shader_MetalPtr mtlVertexShader = std::static_pointer_cast<Shader_Metal>(vertexShader);
        Shader_MetalPtr mtlFragmentShader = std::static_pointer_cast<Shader_Metal>(fragmentShader);
        
        void* vertexDesc = nullptr;
        if (bindParams.geom)
        {
            const GeometryDataPtr& data = bindParams.geom->getGeometryData();
            if (data.get())
            {
                const Attribute* attribData = decl.getAttribData();
                if (attribData)
                {
                    vertexDesc = MTLCreateVertexDescriptor(
                        attribData,
                        (unsigned) decl.getNumAttributes(),
                        (unsigned) decl.getStride());
                }
            }
        }
        
        void* pipeDesc = MTLCreatePipelineDescriptor(
             tag,
             mtlVertexShader->getShaderPtr(),
             mtlFragmentShader->getShaderPtr(),
             vertexDesc,
             bindParams.channels,
             bindParams.depth,
             bindParams.srcBlend,
             bindParams.dstBlend);
        
        if (param_map)
        {
            (*param_map)[bindParams] = pipeDesc;
        }
        
        return pipeDesc;
    }
    
	void ShaderProgram_Metal::bind(const ShaderBindParams& bindParams, const char* tag)
	{
        if (!bindParams.geom)
        {
            return;
        }
        
        VertexDeclaration use_decl;
        ShaderBindParamMap* param_map = this->getParamMap(bindParams.geom->getGeometryData()->decl, use_decl);
        assert(param_map);
        
        ShaderBindParamMap::iterator it = (*param_map).find(bindParams);
        if (it == (*param_map).end())
        {
            createPipelineDescInternal(bindParams, param_map, use_decl, tag);
        }
	}
    
    void* ShaderProgram_Metal::getBufferOffset(const std::string& name, ShaderType shaderType, void* dst)
    {
        Shader_MetalPtr targetShader = std::static_pointer_cast<Shader_Metal>(this->getShader(shaderType));
        if (targetShader.get())
        {
            Shader_Metal::UniformInfoMap::const_iterator it = targetShader->uniformMap.find(name);
            if (it != targetShader->uniformMap.end())
            {
                const Shader_Metal::UniformInfo& info = it->second;
                return PTR_ADD(dst, info.offset);
            }
        }
        return nullptr;
    }
}
