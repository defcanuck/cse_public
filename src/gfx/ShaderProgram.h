#pragma once

#include "ClassDef.h"
#include "gfx/Attribute.h"
#include "math/GLM.h"
#include "gfx/Shader.h"
#include "gfx/ShaderParams.h"
#include "gfx/ShaderBinding.h"

namespace cs
{
    class Geometry;
    
	CLASS_DEFINITION(ShaderProgram)
    
	public:

		ShaderProgram() :
			linked(false) { }

		virtual ~ShaderProgram() { }

		virtual void setUniformValueArray(const std::string& name, float32* value, uint32 count = 0, uint32 precision = 0, void* dst = nullptr) = 0;
		virtual void setUniformValueArray(const std::string& name, int32* value, uint32 count = 0, void* dst = nullptr) = 0;
		virtual void setUniformValueArray(const std::string& name, vec2* value, uint32 count = 0, void* dst = nullptr) = 0;
		virtual void setUniformValueArray(const std::string& name, vec3* value, uint32 count = 0, void* dst = nullptr) = 0;
		virtual void setUniformValueArray(const std::string& name, vec4* value, uint32 count = 0, void* dst = nullptr) = 0;
		virtual void setUniformValueArray(const std::string& name, mat4* value, uint32 count = 0, void* dst = nullptr) = 0;

		virtual void addShader(ShaderPtr& shader);
        virtual ShaderPtr getShader(ShaderType type);
		virtual bool bindAttributeLocation(const std::string& name, AttributeType type) = 0;
		virtual void link(const ShaderParams* params) = 0;
        virtual void bind(const ShaderBindParams& bindParams, const char* tag = nullptr) = 0;

        virtual void* getPipelineDescriptor(const ShaderBindParams& bindParams) { return nullptr; }

	protected:

		bool linked;
		std::map<ShaderType, ShaderPtr> shaders;
	};
}
