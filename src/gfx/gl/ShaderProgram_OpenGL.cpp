#include "PCH.h"

#include "gfx/gl/ShaderProgram_OpenGL.h"
#include "gfx/gl/Shader_OpenGL.h"

// #define CHECK_UNIFORM_VALIDITY 1
#if defined(CHECK_UNIFORM_VALIDITY) 
	#define UNIFORM_LOCATION_CHECK() assert(location >= 0)
#else
	#define UNIFORM_LOCATION_CHECK() void(0)
#endif

namespace cs
{

	void ShaderProgram_OpenGL::init()
	{
		GL_CHECK(this->program = glCreateProgram());
	}

	void ShaderProgram_OpenGL::free()
	{
		ShaderBindParams bindParams;
		this->bind(bindParams);
		for (auto& it : this->shaders)
		{
			const Shader_OpenGLPtr& shader = std::static_pointer_cast<Shader_OpenGL>(it.second);
			GLuint handle = shader->getHandle();
			if (handle > 0)
				GL_CHECK(glDetachShader(this->program, handle));
		}
		
		this->shaders.clear();
		GL_CHECK(glDeleteProgram(this->program));
		
		this->program = 0;
		this->linked = false;
	}

	bool ShaderProgram_OpenGL::valid() const
	{
		return this->program > 0 && this->linked;
	}

	bool ShaderProgram_OpenGL::bindAttributeLocation(const std::string& name, AttributeType type)
	{
		if (this->program <= 0)
			return false;

		GL_CHECK(glBindAttribLocation(this->program, (GLuint) type, name.c_str()));
		attributes[type] = name;
        return true;
	}

	GLint ShaderProgram_OpenGL::getUniformLocation(const std::string& name)
	{
		UniformLocations::iterator it;
		if ((it = this->uniforms.find(name)) != this->uniforms.end())
			return it->second;

		GLint loc;
		GL_CHECK(loc = glGetUniformLocation(this->program, name.c_str()));
		this->uniforms[name] = loc;

		return loc;
	}

	void ShaderProgram_OpenGL::setUniformValueArray(const std::string& name, float32* value, uint32 count, uint32 precision, void* dst)
	{
		if (!this->valid())
			return;

		int location = this->getUniformLocation(name);
		UNIFORM_LOCATION_CHECK();
		GL_CHECK(glUniform1fv(location, count, reinterpret_cast<GLfloat*>(value)));
	}

	void ShaderProgram_OpenGL::setUniformValueArray(const std::string& name, int32* value, uint32 count, void* dst)
	{
		if (!this->valid())
			return;

		int location = this->getUniformLocation(name);
		UNIFORM_LOCATION_CHECK();
		GL_CHECK(glUniform1iv(location, count, reinterpret_cast<GLint*>(value)));
	}

	void ShaderProgram_OpenGL::setUniformValueArray(const std::string& name, vec2* value, uint32 count, void* dst)
	{
		if (!this->valid())
			return;

		int location = this->getUniformLocation(name);
		UNIFORM_LOCATION_CHECK();
		GL_CHECK(glUniform2fv(location, count, reinterpret_cast<GLfloat*>(value)));
	}

	void ShaderProgram_OpenGL::setUniformValueArray(const std::string& name, vec3* value, uint32 count, void* dst)
	{
		if (!this->valid())
			return;
	
		int location = this->getUniformLocation(name);
		UNIFORM_LOCATION_CHECK();
		GL_CHECK(glUniform3fv(location, count, reinterpret_cast<GLfloat*>(value)));
	}

	void ShaderProgram_OpenGL::setUniformValueArray(const std::string& name, vec4* value, uint32 count, void* dst)
	{
		if (!this->valid())
			return;

		int location = this->getUniformLocation(name);
		UNIFORM_LOCATION_CHECK();
		GL_CHECK(glUniform4fv(location, count, reinterpret_cast<GLfloat*>(value)));
	}

	void ShaderProgram_OpenGL::setUniformValueArray(const std::string& name, mat4* value, uint32 count, void* dst)
	{
		if (!this->valid())
			return;

		GL_CHECK(glUniformMatrix4fv(this->getUniformLocation(name), count, false, reinterpret_cast<GLfloat*>(value)));
	}

	void ShaderProgram_OpenGL::link(const ShaderParams* params)
	{
		// attach shaders
		for (auto& it : this->shaders)
		{
			const Shader_OpenGLPtr& shader = std::static_pointer_cast<Shader_OpenGL>(it.second);
			GL_CHECK(glAttachShader(this->program, shader->getHandle()));
		}

		// link
		GLint success = 0;
		GL_CHECK(glLinkProgram(this->program));
		GL_CHECK(glGetProgramiv(this->program, GL_LINK_STATUS, &success));

		if (!(this->linked = !(success == GL_FALSE)))
		{

			GLint maxLength = 0;
			GL_CHECK(glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &maxLength));
            
            if (maxLength > 0)
            {
                //The maxLength includes the NULL character
                GLchar* infoLog = new GLchar[maxLength];
                GL_CHECK(glGetProgramInfoLog(this->program, maxLength, &maxLength, infoLog));

                log::print(LogError, infoLog);
                delete[] infoLog;
            }
            else
            {
                log::print(LogError, "Unknown Shader Linking Error!");
            }
			this->free();
		}
	}

	void ShaderProgram_OpenGL::bind(const ShaderBindParams& bindParams, const char* tag)
	{
		if (!this->valid())
			return;

		GL_CHECK(glUseProgram(this->program));
	}

}
