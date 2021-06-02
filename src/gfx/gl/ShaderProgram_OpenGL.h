#pragma once

#include "ClassDef.h"
#include "gfx/ShaderProgram.h"
#include "gfx/ShaderBinding.h"
#include "gfx/gl/OpenGL.h"

#include <unordered_map>

namespace cs
{
	CLASS_DEFINITION_DERIVED(ShaderProgram_OpenGL, ShaderProgram)
	public:

		ShaderProgram_OpenGL() : ShaderProgram() { this->init(); }
		virtual ~ShaderProgram_OpenGL() { this->free(); }

		virtual void setUniformValueArray(const std::string& name, float32* value, uint32 count = 0, uint32 precision = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, int32* value, uint32 count = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, vec2* value, uint32 count = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, vec3* value, uint32 count = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, vec4* value, uint32 count = 0, void* dst = nullptr);
		virtual void setUniformValueArray(const std::string& name, mat4* value, uint32 count = 0, void* dst = nullptr);

		virtual bool bindAttributeLocation(const std::string& name, AttributeType type);
		virtual void link(const ShaderParams* params);
		virtual void bind(const ShaderBindParams& params, const char* tag = nullptr);

	private:

		void init();
		void free();
		bool valid() const;

		GLint getUniformLocation(const std::string& name);

		GLuint program;

		typedef std::map<int32, std::string> AttributeLocations;
		AttributeLocations attributes;

		typedef std::unordered_map<std::string, GLint> UniformLocations;
		UniformLocations uniforms;
	};
}
