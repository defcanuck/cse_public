#pragma once

#include "ClassDef.h"
#include "gfx/ShaderProgram.h"
#include "gfx/VertexDeclaration.h"
#include <unordered_map>

namespace cs
{
	CLASS_DEFINITION_DERIVED(ShaderProgram_Metal, ShaderProgram)
	public:

		ShaderProgram_Metal()
            : ShaderProgram()
        {
            this->init();
        }
    
		virtual ~ShaderProgram_Metal()
        {
            this->free();
        }

		virtual void setUniformValueArray(const std::string& name, float32* value, uint32 count = 0, uint32 precision = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, int32* value, uint32 count = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, vec2* value, uint32 count = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, vec3* value, uint32 count = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, vec4* value, uint32 count = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, mat4* value, uint32 count = 0, void* dst = nullptr);

		virtual bool bindAttributeLocation(const std::string& name, AttributeType type);
		virtual void link(const ShaderParams* params);
		virtual void bind(const ShaderBindParams& bindParams, const char* tag = nullptr);
        virtual void* getPipelineDescriptor(const ShaderBindParams& bindParams);
    
        const std::map<uint32, uint32>* getTextureStageMap() { return &this->textureStageMap; }
    
        virtual void* getBufferOffset(const std::string& name, ShaderType shaderType, void* dst);
    
	private:

		void init();
		void free();
		bool valid() const;
    
        typedef std::map<AttributeType, int32> AttributeLocations;
        AttributeLocations attributes;
        
        typedef std::map<ShaderBindParams, void*, ShaderBindParamsCompare> ShaderBindParamMap;
        typedef std::map<VertexDeclaration, ShaderBindParamMap, VertexDeclarationCompare> ShaderVertexParamMap;
        ShaderVertexParamMap pipelineDescriptor;

        ShaderBindParamMap* getParamMap(const VertexDeclaration& decl, VertexDeclaration& out_decl);
        void* createPipelineDescInternal(const ShaderBindParams& bindParams, ShaderBindParamMap* param_map, const VertexDeclaration& decl, const char* tag = "");

        std::map<uint32, uint32> textureStageMap;

	};
}
